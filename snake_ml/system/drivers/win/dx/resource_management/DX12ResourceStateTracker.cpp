// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12ResourceStateTracker.h"

#include "system/drivers/win/dx/pipeline/DX12CommandList.h"
#include "system/drivers/win/dx/resource/DX12Resource.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

std::mutex DX12ResourceStateTracker::s_globalMutex;
bool DX12ResourceStateTracker::s_isLocked = false;
DX12ResourceStateTracker::ResourceStateMap DX12ResourceStateTracker::s_globalResourceState;

void DX12ResourceStateTracker::ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier)
{
	if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
	{
		const D3D12_RESOURCE_TRANSITION_BARRIER& transitionBarrier = barrier.Transition;

		// First check if there is already a known "final" state for the given resource.
		// If there is, the resource has been used on the command list before and
		// already has a known state within the command list execution.
		const auto iter = m_finalResourceState.find(transitionBarrier.pResource);
		if (iter != m_finalResourceState.end())
		{
			auto& resourceState = iter->second;
			// If the known final state of the resource is different...
			if (transitionBarrier.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
				!resourceState.m_subresourceState.empty())
			{
				// First transition all of the subresources if they are different than the StateAfter.
				for (auto subresourceState : resourceState.m_subresourceState)
				{
					if (transitionBarrier.StateAfter != subresourceState.second)
					{
						D3D12_RESOURCE_BARRIER newBarrier = barrier;
						newBarrier.Transition.Subresource = subresourceState.first;
						newBarrier.Transition.StateBefore = subresourceState.second;
						m_resourceBarriers.push_back(newBarrier);
					}
				}
			}
			else
			{
				auto finalState = resourceState.GetSubresourceState(transitionBarrier.Subresource);
				if (transitionBarrier.StateAfter != finalState)
				{
					// Push a new transition barrier with the correct before state.
					D3D12_RESOURCE_BARRIER newBarrier = barrier;
					newBarrier.Transition.StateBefore = finalState;
					m_resourceBarriers.push_back(newBarrier);
				}
			}
		}
		else // In this case, the resource is being used on the command list for the first time. 
		{
			// Add a pending barrier. The pending barriers will be resolved
			// before the command list is executed on the command queue.
			m_pendingResourceBarriers.push_back(barrier);
		}

		// Push the final known state (possibly replacing the previously known state for the subresource).
		m_finalResourceState[transitionBarrier.pResource].SetSubresourceState(transitionBarrier.Subresource, transitionBarrier.StateAfter);
	}
	else
	{
		// Just push non-transition barriers to the resource barriers array.
		m_resourceBarriers.push_back(barrier);
	}
}

void DX12ResourceStateTracker::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource)
{
	if (resource)
	{
		ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COMMON, stateAfter, subResource));
	}
}

void DX12ResourceStateTracker::TransitionResource(const DX12Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource)
{
	TransitionResource(resource.GetD3D12Resource().Get(), stateAfter, subResource);
}

void DX12ResourceStateTracker::UAVBarrier(const DX12Resource* resource)
{
	ID3D12Resource* pResource = resource != nullptr ? resource->GetD3D12Resource().Get() : nullptr;

	ResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(pResource));
}

void DX12ResourceStateTracker::AliasBarrier(const DX12Resource* resourceBefore, const DX12Resource* resourceAfter)
{
	ID3D12Resource* pResourceBefore = resourceBefore != nullptr ? resourceBefore->GetD3D12Resource().Get() : nullptr;
	ID3D12Resource* pResourceAfter = resourceAfter != nullptr ? resourceAfter->GetD3D12Resource().Get() : nullptr;

	ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Aliasing(pResourceBefore, pResourceAfter));
}

uint32_t DX12ResourceStateTracker::FlushPendingResourceBarriers(DX12CommandList& commandList)
{
	ASSERT(s_isLocked, "DX12ResourceStateTracker::FlushPendingResourceBarriers mutex has to be locked!");

	// Resolve the pending resource barriers by checking the global state of the 
	// (sub)resources. Add barriers if the pending state and the global state do
	//  not match.
	ResourceBarriers resourceBarriers;
	// Reserve enough space (worst-case, all pending barriers).
	resourceBarriers.reserve(m_pendingResourceBarriers.size());

	for (auto pendingBarrier : m_pendingResourceBarriers)
	{
		if (pendingBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)  // Only transition barriers should be pending...
		{
			auto pendingTransition = pendingBarrier.Transition;

			const auto& iter = s_globalResourceState.find(pendingTransition.pResource);
			if (iter != s_globalResourceState.end())
			{
				// If all subresources are being transitioned, and there are multiple
				// subresources of the resource that are in a different state...
				auto& resourceState = iter->second;
				if (pendingTransition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
					!resourceState.m_subresourceState.empty())
				{
					// Transition all subresources
					for (auto subresourceState : resourceState.m_subresourceState)
					{
						if (pendingTransition.StateAfter != subresourceState.second)
						{
							D3D12_RESOURCE_BARRIER newBarrier = pendingBarrier;
							newBarrier.Transition.Subresource = subresourceState.first;
							newBarrier.Transition.StateBefore = subresourceState.second;
							resourceBarriers.push_back(newBarrier);
						}
					}
				}
				else
				{
					// No (sub)resources need to be transitioned. Just add a single transition barrier (if needed).
					auto globalState = (iter->second).GetSubresourceState(pendingTransition.Subresource);
					if (pendingTransition.StateAfter != globalState)
					{
						// Fix-up the before state based on current global state of the resource.
						pendingBarrier.Transition.StateBefore = globalState;
						resourceBarriers.push_back(pendingBarrier);
					}
				}
			}
		}
	}

	UINT numBarriers = static_cast<UINT>(resourceBarriers.size());
	if (numBarriers > 0)
	{
		auto d3d12CommandList = commandList.GetGraphicsCommandList();
		d3d12CommandList->ResourceBarrier(numBarriers, resourceBarriers.data());
	}

	m_pendingResourceBarriers.clear();

	return numBarriers;
}

void DX12ResourceStateTracker::FlushResourceBarriers(DX12CommandList& commandList)
{
	UINT numBarriers = static_cast<UINT>(m_resourceBarriers.size());
	if (numBarriers > 0)
	{
		auto d3d12CommandList = commandList.GetGraphicsCommandList();
		d3d12CommandList->ResourceBarrier(numBarriers, m_resourceBarriers.data());
		m_resourceBarriers.clear();
	}
	
}

void DX12ResourceStateTracker::CommitFinalResourceStates()
{
	ASSERT(s_isLocked, "DX12ResourceStateTracker::CommitFinalResourceStates mutex has to be locked!");

	// Commit final resource states to the global resource state array (map).
	for (const auto& resourceState : m_finalResourceState)
	{
		s_globalResourceState[resourceState.first] = resourceState.second;
	}

	m_finalResourceState.clear();
}

void DX12ResourceStateTracker::Reset()
{
	// Reset the pending, current, and final resource states.
	m_pendingResourceBarriers.clear();
	m_resourceBarriers.clear();
	m_finalResourceState.clear();
}

void DX12ResourceStateTracker::Lock()
{
	s_globalMutex.lock();
	s_isLocked = true;
}

void DX12ResourceStateTracker::Unlock()
{
	s_globalMutex.unlock();
	s_isLocked = false;
}

void DX12ResourceStateTracker::AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
{
	if (resource != nullptr)
	{
		std::lock_guard<std::mutex> lock(s_globalMutex);
		s_globalResourceState[resource].SetSubresourceState(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state);
	}
}

void DX12ResourceStateTracker::RemoveGlobalResourceState(ID3D12Resource* resource)
{
	if (resource != nullptr)
	{
		std::lock_guard<std::mutex> lock(s_globalMutex);
		s_globalResourceState.erase(resource);
	}
}

DX12ResourceStateTracker::ResourceState::ResourceState(D3D12_RESOURCE_STATES state)
	: m_state(state)
{
}

void DX12ResourceStateTracker::ResourceState::SetSubresourceState(UINT subresource, D3D12_RESOURCE_STATES state)
{
	if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
	{
		m_state = state;
		m_subresourceState.clear();
	}
	else
	{
		m_subresourceState[subresource] = state;
	}
}

D3D12_RESOURCE_STATES DX12ResourceStateTracker::ResourceState::GetSubresourceState(UINT subresource) const
{
	D3D12_RESOURCE_STATES state = m_state;
	const auto iter = m_subresourceState.find(subresource);
	if (iter != m_subresourceState.end())
	{
		state = iter->second;
	}
	return state;
}

}
#endif
}
}