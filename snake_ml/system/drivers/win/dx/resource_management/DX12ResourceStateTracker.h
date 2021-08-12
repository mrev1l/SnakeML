#pragma once
#pragma region (copyright)
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
  *  @file ResourceStateTracker.h
  *  @date October 24, 2018
  *  @author Jeremiah van Oosten
  *
  *  @brief Tracks the known state of a (sub)resource within a command list.
  *
  *  The ResourceStateTracker tracks the known state of a (sub)resource within a command list.
  *  It is often difficult (or impossible) to know the current state of a (sub)resource if
  *  it is being used in multiple command lists. For example when doing shadow mapping,
  *  a depth buffer resource is being used as a depth-stencil view in a command list
  *  that is generating the shadow map for a light source, but needs to be used as
  *  a shader-resource view in a command list that is performing shadow mapping. If
  *  the command lists are being generated in separate threads, the exact state of the
  *  resource can't be guaranteed at the moment it is used in the command list.
  *  The ResourceStateTracker class is intended to be used within a command list
  *  to track the state of the resource as it is known within that command list.
  *
  *  @see https://youtu.be/nmB2XMasz2o
  *  @see https://msdn.microsoft.com/en-us/library/dn899226(v=vs.85).aspx#implicit_state_transitions
  */
#pragma endregion

#include "lib_includes/directX_includes.h"

#include <map>
#include <mutex>
#include <unordered_map>

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

class DX12CommandList;
class DX12Resource;

class DX12ResourceStateTracker
{
public:
	DX12ResourceStateTracker() = default;
	virtual ~DX12ResourceStateTracker() = default;

	/**
	 * Push a resource barrier to the resource state tracker.
	 *
	 * @param barrier The resource barrier to push to the resource state tracker.
	 */
	void ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier);

	/**
	 * Push a transition resource barrier to the resource state tracker.
	 *
	 * @param resource The resource to transition.
	 * @param stateAfter The state to transition the resource to.
	 * @param subResource The subresource to transition. By default, this is D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
	 * which indicates that all subresources should be transitioned to the same state.
	 */
	void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
	void TransitionResource(const DX12Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	/**
	 * Push a UAV resource barrier for the given resource.
	 *
	 * @param resource The resource to add a UAV barrier for. Can be NULL which
	 * indicates that any UAV access could require the barrier.
	 */
	void UAVBarrier(const DX12Resource* resource = nullptr);

	/**
	 * Push an aliasing barrier for the given resource.
	 *
	 * @param beforeResource The resource currently occupying the space in the heap.
	 * @param afterResource The resource that will be occupying the space in the heap.
	 *
	 * Either the beforeResource or the afterResource parameters can be NULL which
	 * indicates that any placed or reserved resource could cause aliasing.
	 */
	void AliasBarrier(const DX12Resource* resourceBefore = nullptr, const DX12Resource* resourceAfter = nullptr);

	/**
	 * Flush any pending resource barriers to the command list.
	 *
	 * @return The number of resource barriers that were flushed to the command list.
	 */
	uint32_t FlushPendingResourceBarriers(DX12CommandList& commandList);

	/**
	 * Flush any (non-pending) resource barriers that have been pushed to the resource state
	 * tracker.
	 */
	void FlushResourceBarriers(DX12CommandList& commandList);

	/**
	 * Commit final resource states to the global resource state map.
	 * This must be called when the command list is closed.
	 */
	void CommitFinalResourceStates();

	/**
	 * Reset state tracking. This must be done when the command list is reset.
	 */
	void Reset();

	/**
	 * The global state must be locked before flushing pending resource barriers
	 * and committing the final resource state to the global resource state.
	 * This ensures consistency of the global resource state between command list
	 * executions.
	 */
	static void Lock();

	/**
	 * Unlocks the global resource state after the final states have been committed
	 * to the global resource state array.
	 */
	static void Unlock();

	/**
	 * Add a resource with a given state to the global resource state array (map).
	 * This should be done when the resource is created for the first time.
	 */
	static void AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);

	/**
	 * Remove a resource from the global resource state array (map).
	 * This should only be done when the resource is destroyed.
	 */
	static void RemoveGlobalResourceState(ID3D12Resource* resource);

private:
	// An array (vector) of resource barriers.
	using ResourceBarriers = std::vector<D3D12_RESOURCE_BARRIER>;

	// Tracks the state of a particular resource and all of its subresources.
	struct ResourceState
	{
		// Initialize all of the subresources within a resource to the given state.
		explicit ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON);

		// Set a subresource to a particular state.
		void SetSubresourceState(UINT subresource, D3D12_RESOURCE_STATES state);

		// Get the state of a (sub)resource within the resource.
		// If the specified subresource is not found in the SubresourceState array (map)
		// then the state of the resource (D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) is
		// returned.
		D3D12_RESOURCE_STATES GetSubresourceState(UINT subresource) const;

		// If the SubresourceState array (map) is empty, then the State variable defines 
		// the state of all of the subresources.
		D3D12_RESOURCE_STATES m_state;
		std::map<UINT, D3D12_RESOURCE_STATES> m_subresourceState;
	};

	using ResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;

	// Pending resource transitions are committed before a command list
	// is executed on the command queue. This guarantees that resources will
	// be in the expected state at the beginning of a command list.
	ResourceBarriers m_pendingResourceBarriers;

	// Resource barriers that need to be committed to the command list.
	ResourceBarriers m_resourceBarriers;

	// The final (last known state) of the resources within a command list.
	// The final resource state is committed to the global resource state when the 
	// command list is closed but before it is executed on the command queue.
	ResourceStateMap m_finalResourceState;

	// The global resource state array (map) stores the state of a resource
	// between command list execution.
	static ResourceStateMap s_globalResourceState;

	// The mutex protects shared access to the GlobalResourceState map.
	static std::mutex s_globalMutex;
	static bool s_isLocked;
};

}
#endif
}
