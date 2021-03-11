// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12Resource.h"

#include "DX12Driver.h"
#include "DX12ResourceStateTracker.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{
DX12Resource::DX12Resource(const std::wstring& name)
	: m_resourceName(name)
{
}

DX12Resource::DX12Resource(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue, const std::wstring& name)
{
	auto device = ((DX12Driver*)DX12Driver::GetInstance())->GetD3D12Device();

	if (clearValue)
	{
		m_d3d12ClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*clearValue);
	}

	CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
	dxutils::ThrowIfFailed(device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		m_d3d12ClearValue.get(),
		IID_PPV_ARGS(&m_d3d12Resource)
	));

	DX12ResourceStateTracker::AddGlobalResourceState(m_d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON);

	SetName(name);
}

DX12Resource::DX12Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const std::wstring& name)
	: m_d3d12Resource(resource)
{
	SetName(name);
}

DX12Resource::DX12Resource(const DX12Resource& copy)
	: m_d3d12Resource(copy.m_d3d12Resource)
	, m_resourceName(copy.m_resourceName)
	, m_d3d12ClearValue(std::make_unique<D3D12_CLEAR_VALUE>(*copy.m_d3d12ClearValue))
{
}

DX12Resource::DX12Resource(DX12Resource&& copy)
	: m_d3d12Resource(std::move(copy.m_d3d12Resource))
	, m_resourceName(std::move(copy.m_resourceName))
	, m_d3d12ClearValue(std::move(copy.m_d3d12ClearValue))
{
}

DX12Resource& DX12Resource::operator=(const DX12Resource& other)
{
	if (this != &other)
	{
		m_d3d12Resource = other.m_d3d12Resource;
		m_resourceName = other.m_resourceName;
		if (other.m_d3d12ClearValue)
		{
			m_d3d12ClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*other.m_d3d12ClearValue);
		}
	}

	return *this;
}

DX12Resource& DX12Resource::operator=(DX12Resource&& other)
{
	if (this != &other)
	{
		m_d3d12Resource = other.m_d3d12Resource;
		m_resourceName = other.m_resourceName;
		m_d3d12ClearValue = std::move(other.m_d3d12ClearValue);

		other.m_d3d12Resource.Reset();
		other.m_resourceName.clear();
	}

	return *this;
}

D3D12_RESOURCE_DESC DX12Resource::GetD3D12ResourceDesc() const
{
	D3D12_RESOURCE_DESC resDesc = {};
	if (m_d3d12Resource)
	{
		resDesc = m_d3d12Resource->GetDesc();
	}

	return resDesc;
}

void DX12Resource::SetD3D12Resource(Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource, const D3D12_CLEAR_VALUE* clearValue)
{
	m_d3d12Resource.Swap(d3d12Resource);
	if (m_d3d12ClearValue)
	{
		m_d3d12ClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*clearValue);
	}
	else
	{
		m_d3d12ClearValue.reset();
	}
	SetName(m_resourceName);
}

void DX12Resource::SetName(const std::wstring& name)
{
	m_resourceName = name;
	if (m_d3d12Resource && !m_resourceName.empty())
	{
		m_d3d12Resource->SetName(m_resourceName.c_str());
	}
}

void DX12Resource::Reset()
{
	m_d3d12Resource.Reset();
	m_d3d12ClearValue.reset();
}

}
#endif
}
}