#include "../include/devdx12.h"
#include "../include/meshdx12.h"
namespace RenderDemo
{
	Devicedx12::Devicedx12()
	{

	}

	Devicedx12::~Devicedx12()
	{
		doDestroy();
	}

	bool Devicedx12::doCreate(void* pInitData /* = nullptr */)
	{
#ifdef _DEBUG
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
#endif
		//const UINT FrameCount = 3;
		const UINT NumContexts = 3;
		const UINT NumLights = 3;		// Keep this in sync with "shaders.hlsl".
		const UINT TitleThrottle = 200;	// Only update the titlebar every X number of frames.
										// Command list submissions from main thread.
		const int CommandListCount = 3;
		const int CommandListPre = 0;
		const int CommandListMid = 1;
		const int CommandListPost = 2;

		const UINT m_width = 1280;
		const UINT m_height = 720;
		HWND hWnd = (HWND)((IRender::InitData*)pInitData)->mhWnd;
		const bool bFullscreen = ((IRender::InitData*)pInitData)->mbFullscreen;

		HRESULT hr = S_FALSE;
		ComPtr<IDXGIFactory4> factory;
		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

		hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
		if (S_OK != hr)
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
			hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
		}
		//
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
		//
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.BufferDesc.Width = m_width;
		swapChainDesc.BufferDesc.Height = m_height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = TRUE;

		ComPtr<IDXGISwapChain> swapChain;
		hr = factory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, &swapChain);
		ThrowIfFailed(swapChain.As(&m_swapChain));
		fullscreen(bFullscreen);

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Create descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = FrameCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
			m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
			// Create a RTV for each frame.
			for (UINT n = 0; n < FrameCount; n++)
			{
				ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
				m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, m_rtvDescriptorSize);
			}

			// Describe and create a depth stencil view (DSV) descriptor heap.
			// Each frame has its own depth stencils (to write shadows onto) 
			// and then there is one for the scene itself.
			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
			dsvHeapDesc.NumDescriptors = 1 + FrameCount * 1;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

			// Describe and create a shader resource view (SRV) and constant 
			// buffer view (CBV) descriptor heap.  Heap layout: null views, 
			// object diffuse + normal textures views, frame 1's shadow buffer, 
			// frame 1's 2x constant buffer, frame 2's shadow buffer, frame 2's 
			// 2x constant buffers, etc...
			const UINT nullSrvCount = 2;		// Null descriptors are needed for out of bounds behavior reads.
			const UINT cbvCount = FrameCount * 2;
			const UINT srvCount = 128;//_countof(SampleAssets::Textures) + (FrameCount * 1);
			D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
			cbvSrvHeapDesc.NumDescriptors = nullSrvCount + cbvCount + srvCount;
			cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_cbvSrvHeap)));

			// Describe and create a sampler descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
			samplerHeapDesc.NumDescriptors = 2;		// One clamp and one wrap sampler.
			samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ThrowIfFailed(m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_samplerHeap)));
		}

		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

		createTaskCmd();
		return true;
	}

	void Devicedx12::doDestroy()
	{
		std::map<std::string, ID3D12PipelineState*>::iterator it;
		for (it = m_psoMap.begin(); it != m_psoMap.end(); ++it)
		{
			it->second->Release();
		}
		m_psoMap.clear();
		TaskCmdMaindx12::destroyMem<TaskCmdMaindx12>((TaskCmdMaindx12*)mpMainTaskCmd);
	}

	void Devicedx12::fullscreen(bool bFullscreen)
	{
		m_swapChain->SetFullscreenState(bFullscreen, nullptr);
	}

	void Devicedx12::frameClear()
	{
		//mpMainTaskCmd->render(0.f,0);
	}

	void Devicedx12::framePresent()
	{
		//mpMainTaskCmd->render(0.f, 1);
		//UINT SyncInterval, UINT Flags
		ThrowIfFailed(m_swapChain->Present(0, 0));
	}

	//void Devicedx12::present(UINT SyncInterval, UINT Flags)
	//{
	//	ThrowIfFailed(m_swapChain->Present(0,0));
	//}

	void* Devicedx12::createPSO(const std::string& strName)
	{
		// Create an empty root signature.
		{
			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
			rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;
			ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
			ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
		}

		ID3D12PipelineState* pPSO = nullptr;

		// Create the pipeline state, which includes compiling and loading shaders.
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

#ifdef _DEBUG
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ResPath rp;
		//String sf = rp.shaderPath() + String("shaders.hlsl");
		String sf("F:\\Prestudy\\renderdx12\\x64\\data\\shader\\shaders.hlsl");
		std::wstring wstr;
		//wstr = sf.toWString(wstr);
		////std::wstring wstr;
		//LPCWSTR lpstr = sf.toLPCWStr(wstr);
		//lpstr = wstr.c_str();
		ThrowIfFailed(D3DCompileFromFile(sf.toLPCWStr(wstr), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(sf.toLPCWStr(wstr), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
		psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pPSO)));

		m_psoMap[strName] = pPSO;
		return pPSO;
	}

	void Devicedx12::createTaskCmd()
	{
		TaskCmddx12::InitData initData;
		initData.mbThreaded = false;
		initData.muSubmit = 1;
		initData.mpDevice = this;
		initData.mTemp = createPSO("mainPSO");
		mpMainTaskCmd = TaskCmdMaindx12::createMem<TaskCmdMaindx12>(&initData);
	}

	ID3D12PipelineState* Devicedx12::findPSO(const std::string& strName)
	{
		ID3D12PipelineState* pso = nullptr;
		pso = m_psoMap[strName];
		return pso;
	}

	ID3D12Device* Devicedx12::deviceHW()
	{
		return m_device.Get();
	}

	



}