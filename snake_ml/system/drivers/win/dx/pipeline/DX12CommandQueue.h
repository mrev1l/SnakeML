#pragma once
#include "lib_includes/directX_includes.h"

#include "utils/types/ThreadSafeQueue.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

class DX12CommandList;

class DX12CommandQueue
{
public:
	DX12CommandQueue(D3D12_COMMAND_LIST_TYPE type);
	virtual ~DX12CommandQueue();

	// Get an available command list from the command queue.
	std::shared_ptr<DX12CommandList> GetCommandList();
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const { return m_d3d12CommandQueue; }

	// Execute a command list.
	// Returns the fence value to wait for for this command list.
	uint64_t ExecuteCommandList(std::shared_ptr<DX12CommandList> commandList);
	uint64_t ExecuteCommandLists(const std::vector<std::shared_ptr<DX12CommandList> >& commandLists);

	uint64_t Signal();
	bool IsFenceComplete(uint64_t fenceValue);
	void WaitForFenceValue(uint64_t fenceValue);
	void Flush();

	// Wait for another command queue to finish.
	void Wait(const DX12CommandQueue& other);

private:
	// Keep track of command allocators that are "in-flight"
	// The first member is the fence value to wait for, the second is the 
	// a shared pointer to the "in-flight" command list.
	using CommandListEntry = std::tuple<uint64_t, std::shared_ptr<DX12CommandList> >;

	// Free any command lists that are finished processing on the command queue.
	void ProccessInFlightCommandLists();

	D3D12_COMMAND_LIST_TYPE										m_commandListType;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>					m_d3d12CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence>							m_d3d12Fence;
	std::atomic_uint64_t										m_fenceValue;

	ThreadSafeQueue<CommandListEntry>					m_inFlightCommandLists;
	ThreadSafeQueue<std::shared_ptr<DX12CommandList>>	m_availableCommandLists;

	// A thread to process in-flight command lists.
	std::thread m_processInFlightCommandListsThread;
	std::atomic_bool m_bProcessInFlightCommandLists;
	std::mutex m_processInFlightCommandListsThreadMutex;
	std::condition_variable m_processInFlightCommandListsThreadCV;
};

}
#endif
}
