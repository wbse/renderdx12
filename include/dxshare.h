#pragma once
//#include "../../render/include/userender.h"
#include "inrenderdx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <pix.h>
//#include "d3dx12.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")


namespace RenderDemo
{
	class Swapchain : public IRoot
	{
	public:

	protected:
		bool doCreate(void* pInitData = nullptr) override;
		void doDestroy() override;
	};

}
