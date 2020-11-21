// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12CommandQueue.h"

#include "utils/directX_utils.h"
#include "utils/win_utils.h"

namespace snakeml
{
namespace system
{
namespace win
{

DX12CommandQueue::DX12CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
	: m_fenceValue(0)
	, m_commandListType(type)
	, m_d3d12Device(device)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	dxutils::ThrowIfFailed(m_d3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
	dxutils::ThrowIfFailed(m_d3d12Device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence)));

	m_fenceEvent = WinUtils::CreateEventHandle();
	ASSERT(m_fenceEvent, "Failed to create fence event handle.");
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> DX12CommandQueue::GetD3D12CommandList()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList;

	if (!m_commandAllocatorQueue.empty() && IsFenceComplete(m_commandAllocatorQueue.front().fenceValue))
	{
		commandAllocator = m_commandAllocatorQueue.front().commandAllocator;
		m_commandAllocatorQueue.pop();

		dxutils::ThrowIfFailed(commandAllocator->Reset());
	}
	else
	{
		commandAllocator = CreateCommandAllocator(); // !!!!!!!! Not pushed to the queue ???
	}

	if (!m_commandListQueue.empty())
	{
		commandList = m_commandListQueue.front();
		m_commandListQueue.pop();

		dxutils::ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
	}
	else
	{
		commandList = CreateCommandList(commandAllocator); // !!!!!!!! Not pushed to the queue ???
	}

	// Associate the command allocator with the command list so that it can be
	// retrieved when the command list is executed.
	dxutils::ThrowIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

	return commandList;
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> DX12CommandQueue::GetD3D12CommandQueue() const
{
	return m_d3d12CommandQueue;
}

uint64_t DX12CommandQueue::ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	commandList->Close();

	ID3D12CommandAllocator* commandAllocator;
	UINT dataSize = sizeof(ID3D12CommandAllocator);
	dxutils::ThrowIfFailed(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

	ID3D12CommandList* const ppCommandLists[] = {
		commandList.Get()
	};

	m_d3d12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
	uint64_t fenceValue = Signal();

	m_commandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
	m_commandListQueue.push(commandList);

	// The ownership of the command allocator has been transferred to the ComPtr
	// in the command allocator queue. It is safe to release the reference 
	// in this temporary COM pointer here.
	commandAllocator->Release();

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
		m_d3d12Fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		::WaitForSingleObject(m_fenceEvent, DWORD_MAX);
	}
}

void DX12CommandQueue::Flush()
{
	WaitForFenceValue(Signal());
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> DX12CommandQueue::CreateCommandAllocator()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	dxutils::ThrowIfFailed(m_d3d12Device->CreateCommandAllocator(m_commandListType, IID_PPV_ARGS(&commandAllocator)));

	return commandAllocator;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> DX12CommandQueue::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator)
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList;
	dxutils::ThrowIfFailed(m_d3d12Device->CreateCommandList(0, m_commandListType, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

	return commandList;
}

}
}
}


