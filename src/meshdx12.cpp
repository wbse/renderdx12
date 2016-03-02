#include "../include/meshdx12.h"
#include "../include/meshdx12.h"

namespace RenderDemo
{
	TaskCmddx12::TaskCmddx12()
	{
	}

	TaskCmddx12::~TaskCmddx12()
	{
		destroy();
	}

	bool TaskCmddx12::doCreate(void* pInitData /* = nullptr */)
	{
		TaskCmddx12::InitData* pid = (TaskCmddx12::InitData*)pInitData;
		Devicedx12* pDevice = (Devicedx12*)pid->mpDevice;
		m_device = pDevice->deviceHW();
		ID3D12PipelineState* pso = pDevice->findPSO("mainPSO");
		UINT uSub = pid->muSubmit;
		for (int i = 0; i < muCmdMax; i++)
		{
			if (i < uSub)
			{
				ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocators[i])));
				ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocators[i].Get(), pso, IID_PPV_ARGS(&m_cmdLists[i])));
				// Close these command lists; don't record into them for now.
				ThrowIfFailed(m_cmdLists[i]->Close());
			}
			else
			{
				m_cmdAllocators[i] = nullptr;
				m_cmdLists[i] = nullptr;
			}
		}
		return true;
	}

	void TaskCmddx12::doDestroy()
	{
		for (int i = 0; i < muCmdMax; i++)
		{
			m_cmdAllocators[i] = nullptr;
			m_cmdLists[i] = nullptr;
		}
		m_device = nullptr;
	}

	void TaskCmddx12::update(float delta)
	{
		;
	}

	void TaskCmddx12::render(float delta, UINT uPhase /* = 0 */)
	{
		switch (uPhase)
		{
		case 0:
			phase0();
			break;

		case 1:
			phase1();
			break;

		case 2:
			phase2();
			break;

		case 3:
			phase3();
			break;

		default:
			phase0();
			break;
		};
	}

	void TaskCmddx12::refresh()
	{
		;
	}

	ID3D12GraphicsCommandList*	TaskCmddx12::cmdList(UINT idx /* = 0 */)
	{
		ID3D12GraphicsCommandList* pCmdList = nullptr;
		if (idx >= 0 && idx < muCmdMax)
		{
			pCmdList = m_cmdLists[idx].Get();
		}
		return pCmdList;
	}

	void TaskCmddx12::phase0()
	{

	}

	void TaskCmddx12::phase1()
	{

	}

	void TaskCmddx12::phase2()
	{

	}

	void TaskCmddx12::phase3()
	{

	}





	void TaskCmdMaindx12::phase0()
	{
		m_cmdAllocators[0]->Reset();
		m_cmdLists[0]->Reset(m_cmdAllocators[0].Get(),nullptr);
		//m_cmdLists[0]->ClearRenderTargetView()
		m_cmdLists[0]->Close();
	}

	void TaskCmdMaindx12::phase1()
	{
		m_cmdAllocators[1]->Reset();
		m_cmdLists[1]->Reset(m_cmdAllocators[0].Get(), nullptr);
		//
		m_cmdLists[1]->Close();
	}

	void TaskCmdMaindx12::phase2()
	{

	}

	void TaskCmdMaindx12::phase3()
	{

	}





	
}