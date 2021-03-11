#pragma once
#pragma region(copyright)
/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

 /**
  *  @file DynamicDescriptorHeap.h
  *  @date October 22, 2018
  *  @author Jeremiah van Oosten
  *
  *  @brief The DynamicDescriptorHeap is a GPU visible descriptor heap that allows for
  *  staging of CPU visible descriptors that need to be uploaded before a Draw
  *  or Dispatch command is executed.
  *  The DynamicDescriptorHeap class is based on the one provided by the MiniEngine:
  *  https://github.com/Microsoft/DirectX-Graphics-Samples
  */
#pragma endregion

#include "lib_includes/directX_includes.h"

#include <functional>
#include <queue>

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

class DX12CommandList;
class DX12RootSignature;

class DX12DynamicDescriptorHeap
{
public:
	DX12DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t numDescriptorsPerHeap = 1024);
	virtual ~DX12DynamicDescriptorHeap() = default;

	/**
	 * Stages a contiguous range of CPU visible descriptors.
	 * Descriptors are not copied to the GPU visible descriptor heap until
	 * the CommitStagedDescriptors function is called.
	 */
	void StageDescriptors(uint32_t rootParameterIndex, uint32_t offset, uint32_t numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors);

	/**
	 * Copy all of the staged descriptors to the GPU visible descriptor heap and
	 * bind the descriptor heap and the descriptor tables to the command list.
	 * The passed-in function object is used to set the GPU visible descriptors
	 * on the command list. Two possible functions are:
	 *   * Before a draw	: ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
	 *   * Before a dispatch: ID3D12GraphicsCommandList::SetComputeRootDescriptorTable
	 *
	 * Since the DynamicDescriptorHeap can't know which function will be used, it must
	 * be passed as an argument to the function.
	 */
	void CommitStagedDescriptors(DX12CommandList& commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);
	void CommitStagedDescriptorsForDraw(DX12CommandList& commandList);
	void CommitStagedDescriptorsForDispatch(DX12CommandList& commandList);

	/**
	 * Copies a single CPU visible descriptor to a GPU visible descriptor heap.
	 * This is useful for the
	 *   * ID3D12GraphicsCommandList::ClearUnorderedAccessViewFloat
	 *   * ID3D12GraphicsCommandList::ClearUnorderedAccessViewUint
	 * methods which require both a CPU and GPU visible descriptors for a UAV
	 * resource.
	 *
	 * @param commandList The command list is required in case the GPU visible
	 * descriptor heap needs to be updated on the command list.
	 * @param cpuDescriptor The CPU descriptor to copy into a GPU visible
	 * descriptor heap.
	 *
	 * @return The GPU visible descriptor.
	 */
	D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(DX12CommandList& comandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);

	/**
	 * Parse the root signature to determine which root parameters contain
	 * descriptor tables and determine the number of descriptors needed for
	 * each table.
	 */
	void ParseRootSignature(const DX12RootSignature& rootSignature);

	/**
	 * Reset used descriptors. This should only be done if any descriptors
	 * that are being referenced by a command list has finished executing on the
	 * command queue.
	 */
	void Reset();

private:
	using DescriptorHeapPool = std::queue<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>;
	/**
	 * A structure that represents a descriptor table entry in the root signature.
	 */
	struct DescriptorTableCache
	{
		DescriptorTableCache()
			: numDescriptors(0)
			, baseDescriptor(nullptr)
		{}

		// Reset the table cache.
		void Reset()
		{
			numDescriptors = 0;
			baseDescriptor = nullptr;
		}

		// The number of descriptors in this descriptor table.
		uint32_t numDescriptors;
		// The pointer to the descriptor in the descriptor handle cache.
		D3D12_CPU_DESCRIPTOR_HANDLE* baseDescriptor;
	};

	// Request a descriptor heap if one is available.
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RequestDescriptorHeap();
	// Create a new descriptor heap of no descriptor heap is available.
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap();
	// Compute the number of stale descriptors that need to be copied to GPU visible descriptor heap.
	uint32_t ComputeStaleDescriptorCount() const;

	/**
	 * The maximum number of descriptor tables per root signature.
	 * A 32-bit mask is used to keep track of the root parameter indices that
	 * are descriptor tables.
	 */
	static const uint32_t s_maxDescriptorTables = 32;

	// Describes the type of descriptors that can be staged using this 
	// dynamic descriptor heap.
	// Valid values are:
	//   * D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	//   * D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
	// This parameter also determines the type of GPU visible descriptor heap to 
	// create.
	D3D12_DESCRIPTOR_HEAP_TYPE m_descriptorHeapType;

	// The number of descriptors to allocate in new GPU visible descriptor heaps.
	uint32_t m_numDescriptorsPerHeap;

	// The increment size of a descriptor.
	uint32_t m_descriptorHandleIncrementSize;

	// The descriptor handle cache.
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> m_descriptorHandleCache;

	// Descriptor handle cache per descriptor table.
	DescriptorTableCache m_descriptorTableCache[s_maxDescriptorTables];

	// Each bit in the bit mask represents the index in the root signature
	// that contains a descriptor table.
	uint32_t m_descriptorTableBitMask;

	// Each bit set in the bit mask represents a descriptor table
	// in the root signature that has changed since the last time the 
	// descriptors were copied.
	uint32_t m_staleDescriptorTableBitMask;

	DescriptorHeapPool m_descriptorHeapPool;
	DescriptorHeapPool m_availableDescriptorHeaps;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_currentDescriptorHeap;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_currentGPUDescriptorHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_currentCPUDescriptorHandle;

	uint32_t m_numFreeHandles;
};

}
#endif
}
}