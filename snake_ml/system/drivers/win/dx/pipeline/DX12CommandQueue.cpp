// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12CommandQueue.h"

#include "system/drivers/win/dx/DX12Driver.h"
#include "system/drivers/win/dx/pipeline/DX12CommandList.h"
#include "system/drivers/win/dx/resource_management/DX12ResourceStateTracker.h"

#include "utils/win_utils.h"

namespace snakeml
{
namespace system
{
namespace win
{

DX12CommandQueue::DX12CommandQueue(D3D12_COMMAND_LIST_TYPE type)
	: m_fenceValue(0)
	, m_commandListType(type)
	, m_bProcessInFlightCommandLists(true)
{
	win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	dxutils::ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
	dxutils::ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence)));

	switch (type)
	{
	case D3D12_COMMAND_LIST_TYPE_COPY:
		m_d3d12CommandQueue->SetName(L"Copy Command Queue");
		break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		m_d3d12CommandQueue->SetName(L"Compute Command Queue");
		break;
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		m_d3d12CommandQueue->SetName(L"Direct Command Queue");
		break;
	}

	m_processInFlightCommandListsThread = std::thread(&DX12CommandQueue::ProccessInFlightCommandLists, this);
}

DX12CommandQueue::~DX12CommandQueue()
{
	m_bProcessInFlightCommandLists = false;
	m_processInFlightCommandListsThread.join();
}

std::shared_ptr<DX12CommandList> DX12CommandQueue::GetCommandList()
{
	std::shared_ptr<DX12CommandList> commandList;

	// If there is a command list on the queue.
	if (!m_availableCommandLists.Empty())
	{
		m_availableCommandLists.TryPop(commandList);
	}
	else
	{
		// Otherwise create a new command list.
		commandList = std::make_shared<DX12CommandList>(m_commandListType);
	}

	return commandList;
}

uint64_t DX12CommandQueue::ExecuteCommandList(std::shared_ptr<DX12CommandList> commandList)
{
	return ExecuteCommandLists(std::vector<std::shared_ptr<DX12CommandList> >({ commandList }));
}

uint64_t DX12CommandQueue::ExecuteCommandLists(const std::vector<std::shared_ptr<DX12CommandList>>& commandLists)
{
	DX12ResourceStateTracker::Lock();

	// Command lists that need to put back on the command list queue.
	std::vector<std::shared_ptr<DX12CommandList>> toBeQueued;
	toBeQueued.reserve(commandLists.size() * 2);		// 2x since each command list will have a pending command list.

	// Generate mips command lists.
	std::vector<std::shared_ptr<DX12CommandList>> generateMipsCommandLists;
	generateMipsCommandLists.reserve(commandLists.size());

	// Command lists that need to be executed.
	std::vector<ID3D12CommandList*> d3d12CommandLists;
	d3d12CommandLists.reserve(commandLists.size() * 2); // 2x since each command list will have a pending command list.

	for (auto commandList : commandLists)
	{
		auto pendingCommandList = GetCommandList();
		bool hasPendingBarriers = commandList->Close(*pendingCommandList);
		pendingCommandList->Close();
		// If there are no pending barriers on the pending command list, there is no reason to 
		// execute an empty command list on the command queue.
		if (hasPendingBarriers)
		{
			d3d12CommandLists.push_back(pendingCommandList->GetGraphicsCommandList().Get());
		}
		d3d12CommandLists.push_back(commandList->GetGraphicsCommandList().Get());

		toBeQueued.push_back(pendingCommandList);
		toBeQueued.push_back(commandList);

		auto generateMipsCommandList = commandList->GetGenerateMipsCommandList();
		if (generateMipsCommandList)
		{
			generateMipsCommandLists.push_back(generateMipsCommandList);
		}
	}

	UINT numCommandLists = static_cast<UINT>(d3d12CommandLists.size());
	m_d3d12CommandQueue->ExecuteCommandLists(numCommandLists, d3d12CommandLists.data());
	uint64_t fenceValue = Signal();

	DX12ResourceStateTracker::Unlock();

	// Queue command lists for reuse.
	for (auto commandList : toBeQueued)
	{
		m_inFlightCommandLists.Push({ fenceValue, commandList });
	}

	// If there are any command lists that generate mips then execute those
	// after the initial resource command lists have finished.
	if (generateMipsCommandLists.size() > 0)
	{
		win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();		
		auto computeQueue = dx12Driver->GetDX12CommandQueue(DX12Driver::CommandQueueType::Compute);
		computeQueue->Wait(*this);
		computeQueue->ExecuteCommandLists(generateMipsCommandLists);
	}

	return fenceValue;
}

uint64_t DX12CommandQueue::Signal()
{
	uint64_t fenceValue = ++m_fenceValue;
	m_d3d12CommandQueue->Signal(m_d3d12Fence.Get(), fenceValue);
	return fenceValue;
}

bool DX12CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
	return m_d3d12Fence->GetCompletedValue() >= fenceValue;
}

void DX12CommandQueue::WaitForFenceValue(uint64_t fenceValue)
{
	if (!IsFenceComplete(fenceValue))
	{
		auto eventHandle = winutils::WinUtils::CreateEventHandle();

		// Is this function thread safe?
		m_d3d12Fence->SetEventOnCompletion(fenceValue, eventHandle);
		::WaitForSingleObject(eventHandle, DWORD_MAX);

		::CloseHandle(eventHandle);
	}
}

void DX12CommandQueue::Flush()
{
	std::unique_lock<std::mutex> lock(m_processInFlightCommandListsThreadMutex);
	m_processInFlightCommandListsThreadCV.wait(lock, [this] { return m_inFlightCommandLists.Empty(); });

	// In case the command queue was signaled directly 
	// using the CommandQueue::Signal method then the 
	// fence value of the command queue might be higher than the fence
	// value of any of the executed command lists.
	WaitForFenceValue(m_fenceValue);
}

void DX12CommandQueue::Wait(const DX12CommandQueue& other)
{
	m_d3d12CommandQueue->Wait(other.m_d3d12Fence.Get(), other.m_fenceValue);
}

void DX12CommandQueue::ProccessInFlightCommandLists()
{
	std::unique_lock<std::mutex> lock(m_processInFlightCommandListsThreadMutex, std::defer_lock);

	while (m_bProcessInFlightCommandLists)
	{
		CommandListEntry commandListEntry;

		lock.lock();
		while (m_inFlightCommandLists.TryPop(commandListEntry))
		{
			auto fenceValue = std::get<0>(commandListEntry);
			auto commandList = std::get<1>(commandListEntry);

			WaitForFenceValue(fenceValue);

			commandList->Reset();

			m_availableCommandLists.Push(commandList);
		}
		lock.unlock();
		m_processInFlightCommandListsThreadCV.notify_one();

		std::this_thread::yield();
	}
}

}
}
}


