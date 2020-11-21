#pragma once
#include "lib_includes/directX_includes.h"
#include <queue>

namespace snakeml
{
namespace system
{
namespace win
{

class DX12CommandQueue
{
public:
	DX12CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	virtual ~DX12CommandQueue() = default;

	// Get an available command list from the command queue.
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetD3D12CommandList();
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const;

	// Execute a command list.
	// Returns the fence value to wait for for this command list.
	uint64_t ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	uint64_t Signal();
	bool IsFenceComplete(uint64_t fenceValue);
	void WaitForFenceValue(uint64_t fenceValue);
	void Flush();

private:
	// Keep track of command allocators that are "in-flight"
	struct CommandAllocatorEntry
	{
		uint64_t fenceValue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
	using CommandListQueue = std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>>;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);

	D3D12_COMMAND_LIST_TYPE                     m_commandListType;
	Microsoft::WRL::ComPtr<ID3D12Device2>       m_d3d12Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>  m_d3d12CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence>         m_d3d12Fence;
	HANDLE                                      m_fenceEvent;
	uint64_t                                    m_fenceValue;

	CommandAllocatorQueue                       m_commandAllocatorQueue;
	CommandListQueue                            m_commandListQueue;
};

}
}
}