#pragma once
#include "devdx12.h"

namespace RenderDemo
{
	class RENDERDX12DLL TaskCmddx12 : public IDrawTask
	{
	public:
		struct InitData : public IDrawTask::InitData
		{
			IDevice* mpDevice;
			IPipelineState* mpPSO;
			void* mTemp;
		};

	public:
		TaskCmddx12();
		virtual ~TaskCmddx12();

		void update(float delta) override;
		void render(float delta, UINT uPhase = 0) override;
		void refresh() override;

	public:
		ID3D12GraphicsCommandList*	cmdList(UINT idx = 0);

	protected:
		bool doCreate(void* pInitData = nullptr) override;
		void doDestroy() override;

	protected:
		virtual void phase0();
		virtual void phase1();
		virtual void phase2();
		virtual void phase3();

	protected:
		ID3D12Device* m_device;
		static const UINT muCmdMax = 4;
		ComPtr<ID3D12CommandAllocator> m_cmdAllocators[muCmdMax];
		ComPtr<ID3D12GraphicsCommandList> m_cmdLists[muCmdMax];
	};

	class RENDERDX12DLL TaskCmdMaindx12 : public TaskCmddx12
	{
	protected:
		void phase0() override;
		void phase1() override;
		void phase2() override;
		void phase3() override;
	};



	

}
