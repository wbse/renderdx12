#pragma once
#include "inrenderdx12.h"
#include "dxshare.h"
#include "d3dhelper12.h"

namespace RenderDemo
{
	class RENDERDX12DLL Devicedx12 : public IDevice
	{
	public:
		Devicedx12();
		virtual ~Devicedx12();
		void fullscreen(bool bFullscreen) override;
		void frameClear() override;
		void framePresent() override;
		//void present(UINT SyncInterval,UINT Flags) override;

		void* createPSO(const std::string& strName) override;
		ID3D12PipelineState* findPSO(const std::string& strName);
		void createTaskCmd();
		ID3D12Device* deviceHW();

	protected:
		bool doCreate(void* pInitData = nullptr) override;
		void doDestroy() override;

	private:
		//pipeline
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<IDXGISwapChain3> m_swapChain;

		static const UINT FrameCount = 3;
		ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap;
		ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
		ComPtr<ID3D12PipelineState> m_pipelineState;
		ComPtr<ID3D12PipelineState> m_pipelineStateShadowMap;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		//res
		UINT m_rtvDescriptorSize;
		//sync
		UINT m_frameIndex;

	private:
		std::map<std::string, ID3D12PipelineState*> m_psoMap;
		IDrawTask* mpMainTaskCmd;
	};


	//class RENDERDX12DLL PipeLineStatedx12 : public IPipelineState
	//{
	//public:
	//	PipeLineStatedx12();
	//	virtual ~PipeLineStatedx12();

	//	void* buildPSO(const std::string& strName) override;
	//	void* findPSO(const std::string& strName) override;

	//protected:
	//	bool doCreate(void* pInitData = nullptr) override;
	//	void doDestroy() override;

	//private:
	//	ComPtr<ID3D12Device> m_device;
	//	std::map<std::string, ID3D12PipelineState*> m_psoMap;
	//};

	

}
