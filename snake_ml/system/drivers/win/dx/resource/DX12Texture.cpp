// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12Texture.h"

#include "system/drivers/win/dx/DX12Driver.h"
#include "system/drivers/win/dx/resource_management/DX12ResourceStateTracker.h"

#include <functional>

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

DX12Texture::DX12Texture(TextureUsage textureUsage, const std::wstring& name)
	: DX12Resource(name)
	, m_textureUsage(textureUsage)
{
}

DX12Texture::DX12Texture(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue, 
	TextureUsage textureUsage, const std::wstring& name) 
	: DX12Resource(resourceDesc, clearValue, name)
	, m_textureUsage(textureUsage)
{
	CreateViews();
}

DX12Texture::DX12Texture(Microsoft::WRL::ComPtr<ID3D12Resource> resource, TextureUsage textureUsage, const std::wstring& name)
	: DX12Resource(resource, name)
	, m_textureUsage(textureUsage)
{
	CreateViews();
}

DX12Texture::DX12Texture(const DX12Texture& copy)
	: DX12Resource(copy)
	, m_textureUsage(copy.m_textureUsage)
{
	CreateViews();
}

DX12Texture::DX12Texture(DX12Texture&& copy)
	: DX12Resource(copy)
	, m_textureUsage(copy.m_textureUsage)
{
	CreateViews();
}

DX12Texture& DX12Texture::operator=(const DX12Texture& other)
{
	DX12Resource::operator=(other);
	CreateViews();
	return *this;
}

DX12Texture& DX12Texture::operator=(DX12Texture&& other)
{
	DX12Resource::operator=(other);
	CreateViews();
	return *this;
}

void DX12Texture::Resize(uint32_t width, uint32_t height, uint32_t depthOrArraySize)
{
	// Resource can't be resized if it was never created in the first place.
	if (m_d3d12Resource)
	{
		DX12ResourceStateTracker::RemoveGlobalResourceState(m_d3d12Resource.Get());

		CD3DX12_RESOURCE_DESC resDesc(m_d3d12Resource->GetDesc());

		resDesc.Width = std::max(width, 1u);
		resDesc.Height = std::max(height, 1u);
		resDesc.DepthOrArraySize = depthOrArraySize;

		win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
		auto device = dx12Driver->GetD3D12Device();

		CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
		dxutils::ThrowIfFailed(device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_COMMON,
			m_d3d12ClearValue.get(),
			IID_PPV_ARGS(&m_d3d12Resource)
		));

		// Retain the name of the resource if one was already specified.
		m_d3d12Resource->SetName(m_resourceName.c_str());

		DX12ResourceStateTracker::AddGlobalResourceState(m_d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON);

		CreateViews();
	}
}

void DX12Texture::CreateViews()
{
	if (m_d3d12Resource)
	{
		win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
		auto device = dx12Driver->GetD3D12Device();

		CD3DX12_RESOURCE_DESC desc(m_d3d12Resource->GetDesc());

		D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport;
		formatSupport.Format = desc.Format;
		dxutils::ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT)));

		if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0 &&
			CheckRTVSupport(formatSupport.Support1))
		{
			m_renderTargetView = dx12Driver->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			device->CreateRenderTargetView(m_d3d12Resource.Get(), nullptr, m_renderTargetView.GetDescriptorHandle());
		}
		if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0 &&
			CheckDSVSupport(formatSupport.Support1))
		{
			m_depthStencilView = dx12Driver->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			device->CreateDepthStencilView(m_d3d12Resource.Get(), nullptr, m_depthStencilView.GetDescriptorHandle());
		}
	}

	std::lock_guard<std::mutex> lock(m_shaderResourceViewsMutex);
	std::lock_guard<std::mutex> guard(m_unorderedAccessViewsMutex);

	// SRVs and UAVs will be created as needed.
	m_shaderResourceViews.clear();
	m_unorderedAccessViews.clear();
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Texture::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
{
	std::size_t hash = 0;
	if (srvDesc)
	{
		hash = std::hash<D3D12_SHADER_RESOURCE_VIEW_DESC>::_Do_hash(*srvDesc);
	}

	std::lock_guard<std::mutex> lock(m_shaderResourceViewsMutex);

	auto iter = m_shaderResourceViews.find(hash);
	if (iter == m_shaderResourceViews.end())
	{
		auto srv = CreateShaderResourceView(srvDesc);
		iter = m_shaderResourceViews.insert({ hash, std::move(srv) }).first;
	}

	return iter->second.GetDescriptorHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Texture::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
{
	std::size_t hash = 0;
	if (uavDesc)
	{
		hash = std::hash<D3D12_UNORDERED_ACCESS_VIEW_DESC>::_Do_hash(*uavDesc);
	}

	std::lock_guard<std::mutex> guard(m_unorderedAccessViewsMutex);

	auto iter = m_unorderedAccessViews.find(hash);
	if (iter == m_unorderedAccessViews.end())
	{
		auto uav = CreateUnorderedAccessView(uavDesc);
		iter = m_unorderedAccessViews.insert({ hash, std::move(uav) }).first;
	}

	return iter->second.GetDescriptorHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Texture::GetRenderTargetView() const
{
	return m_renderTargetView.GetDescriptorHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Texture::GetDepthStencilView() const
{
	return m_depthStencilView.GetDescriptorHandle();
}

bool DX12Texture::IsUAVCompatibleFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		//	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SINT:
		return true;
	default:
		return false;
	}
}

bool DX12Texture::IsSRGBFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return true;
	default:
		return false;
	}
}

bool DX12Texture::IsBGRFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return true;
	default:
		return false;
	}
}

bool DX12Texture::IsDepthFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_D16_UNORM:
		return true;
	default:
		return false;
	}
}

DXGI_FORMAT DX12Texture::GetTypelessFormat(DXGI_FORMAT format)
{
	DXGI_FORMAT typelessFormat = format;

	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		typelessFormat = DXGI_FORMAT_R32G32B32A32_TYPELESS;
		break;
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		typelessFormat = DXGI_FORMAT_R32G32B32_TYPELESS;
		break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
		typelessFormat = DXGI_FORMAT_R16G16B16A16_TYPELESS;
		break;
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
		typelessFormat = DXGI_FORMAT_R32G32_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		typelessFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
		break;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
		typelessFormat = DXGI_FORMAT_R10G10B10A2_TYPELESS;
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
		typelessFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS;
		break;
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
		typelessFormat = DXGI_FORMAT_R16G16_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
		typelessFormat = DXGI_FORMAT_R32_TYPELESS;
		break;
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
		typelessFormat = DXGI_FORMAT_R8G8_TYPELESS;
		break;
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
		typelessFormat = DXGI_FORMAT_R16_TYPELESS;
		break;
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
		typelessFormat = DXGI_FORMAT_R8_TYPELESS;
		break;
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		typelessFormat = DXGI_FORMAT_BC1_TYPELESS;
		break;
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		typelessFormat = DXGI_FORMAT_BC2_TYPELESS;
		break;
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		typelessFormat = DXGI_FORMAT_BC3_TYPELESS;
		break;
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		typelessFormat = DXGI_FORMAT_BC4_TYPELESS;
		break;
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
		typelessFormat = DXGI_FORMAT_BC5_TYPELESS;
		break;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		typelessFormat = DXGI_FORMAT_B8G8R8A8_TYPELESS;
		break;
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		typelessFormat = DXGI_FORMAT_B8G8R8X8_TYPELESS;
		break;
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
		typelessFormat = DXGI_FORMAT_BC6H_TYPELESS;
		break;
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		typelessFormat = DXGI_FORMAT_BC7_TYPELESS;
		break;
	}

	return typelessFormat;
}

DX12DescriptorAllocation DX12Texture::CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
{
	win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();
	auto srv = dx12Driver->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	device->CreateShaderResourceView(m_d3d12Resource.Get(), srvDesc, srv.GetDescriptorHandle());

	return srv;
}

DX12DescriptorAllocation DX12Texture::CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
{
	win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();
	auto uav = dx12Driver->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	device->CreateUnorderedAccessView(m_d3d12Resource.Get(), nullptr, uavDesc, uav.GetDescriptorHandle());

	return uav;
}

}
#endif
}
}