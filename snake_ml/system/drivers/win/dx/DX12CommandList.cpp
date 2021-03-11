// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12CommandList.h"

#include "DX12Buffer.h"
#include "DX12Driver.h"
#include "DX12GenerateMipsPSO.h"
#include "DX12DynamicDescriptorHeap.h"
#include "DX12Resource.h"
#include "DX12ResourceStateTracker.h"
#include "DX12RootSignature.h"
#include "DX12Texture.h"
#include "DX12VertexBuffer.h"
#include "DX12UploadBuffer.h"

#include "third_party/win/DirectXTex/DirectXTex/DirectXTex.h"

#include <filesystem>

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

std::map<std::wstring, ID3D12Resource* > DX12CommandList::s_textureCache;
std::mutex DX12CommandList::s_textureCacheMutex;

DX12CommandList::DX12CommandList(D3D12_COMMAND_LIST_TYPE type)
	: m_d3d12CommandListType(type)
{
	auto device = ((DX12Driver*)DX12Driver::GetInstance())->GetD3D12Device();

	dxutils::ThrowIfFailed(device->CreateCommandAllocator(m_d3d12CommandListType, IID_PPV_ARGS(&m_d3d12CommandAllocator)));
	dxutils::ThrowIfFailed(device->CreateCommandList(0, m_d3d12CommandListType, m_d3d12CommandAllocator.Get(),
		nullptr, IID_PPV_ARGS(&m_d3d12CommandList)));

	m_uploadBuffer = std::make_unique<DX12UploadBuffer>();
	m_resourceStateTracker = std::make_unique<DX12ResourceStateTracker>();

	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_dynamicDescriptorHeap[i] = std::make_unique<DX12DynamicDescriptorHeap>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
		m_descriptorHeaps[i] = nullptr;
	}
}

void DX12CommandList::TransitionBarrier(const DX12Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource, bool flushBarriers)
{
	auto d3d12Resource = resource.GetD3D12Resource();
	if (d3d12Resource)
	{
		// The "before" state is not important. It will be resolved by the resource state tracker.
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter, subresource);
		m_resourceStateTracker->ResourceBarrier(barrier);
	}

	if (flushBarriers)
	{
		FlushResourceBarriers();
	}
}

void DX12CommandList::UAVBarrier(const DX12Resource& resource, bool flushBarriers)
{
	auto d3d12Resource = resource.GetD3D12Resource();
	auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(d3d12Resource.Get());

	m_resourceStateTracker->ResourceBarrier(barrier);

	if (flushBarriers)
	{
		FlushResourceBarriers();
	}
}

void DX12CommandList::AliasingBarrier(const DX12Resource& beforeResource, const DX12Resource& afterResource, bool flushBarriers)
{
	auto d3d12BeforeResource = beforeResource.GetD3D12Resource();
	auto d3d12AfterResource = afterResource.GetD3D12Resource();
	auto barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(d3d12BeforeResource.Get(), d3d12AfterResource.Get());

	m_resourceStateTracker->ResourceBarrier(barrier);

	if (flushBarriers)
	{
		FlushResourceBarriers();
	}
}

void DX12CommandList::FlushResourceBarriers()
{
	m_resourceStateTracker->FlushResourceBarriers(*this);
}

void DX12CommandList::CopyResource(DX12Resource& dstRes, const DX12Resource& srcRes)
{
	TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_COPY_SOURCE);

	FlushResourceBarriers();

	m_d3d12CommandList->CopyResource(dstRes.GetD3D12Resource().Get(), srcRes.GetD3D12Resource().Get());

	TrackResource(dstRes);
	TrackResource(srcRes);
}

void DX12CommandList::ResolveSubresource(DX12Resource& dstRes, const DX12Resource& srcRes, uint32_t dstSubresource, uint32_t srcSubresource)
{
	TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_RESOLVE_DEST, dstSubresource);
	TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, srcSubresource);

	FlushResourceBarriers();

	m_d3d12CommandList->ResolveSubresource(dstRes.GetD3D12Resource().Get(), dstSubresource, srcRes.GetD3D12Resource().Get(), srcSubresource, dstRes.GetD3D12ResourceDesc().Format);

	TrackResource(srcRes);
	TrackResource(dstRes);
}

void DX12CommandList::CopyVertexBuffer(DX12VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData)
{
	CopyBuffer(vertexBuffer, numVertices, vertexStride, vertexBufferData);
}

void DX12CommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	m_d3d12CommandList->IASetPrimitiveTopology(primitiveTopology);
}

void DX12CommandList::LoadTextureFromFile(DX12Texture& texture, const std::wstring& fileName, TextureUsage textureUsage)
{
	auto device = ((DX12Driver*)DX12Driver::GetInstance())->GetD3D12Device();

	std::filesystem::path filePath(fileName);
	if (!std::filesystem::exists(filePath))
	{
		throw std::exception("File not found.");
	}

	auto iter = s_textureCache.find(fileName);
	if (iter != s_textureCache.end())
	{
		texture.SetTextureUsage(textureUsage);
		texture.SetD3D12Resource(iter->second);
		texture.CreateViews();
		texture.SetName(fileName);
	}
	else
	{
		DirectX::TexMetadata metadata;
		DirectX::ScratchImage scratchImage;
		Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;

		if (filePath.extension() == ".dds")
		{
			// Use DDS texture loader.
			dxutils::ThrowIfFailed(LoadFromDDSFile(fileName.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratchImage));
		}
		else if (filePath.extension() == ".hdr")
		{
			dxutils::ThrowIfFailed(LoadFromHDRFile(fileName.c_str(), &metadata, scratchImage));
		}
		else if (filePath.extension() == ".tga")
		{
			dxutils::ThrowIfFailed(LoadFromTGAFile(fileName.c_str(), &metadata, scratchImage));
		}
		else
		{
			dxutils::ThrowIfFailed(LoadFromWICFile(fileName.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratchImage));
		}

		if (textureUsage == TextureUsage::Albedo)
		{
			metadata.format = DirectX::MakeSRGB(metadata.format);
		}

		D3D12_RESOURCE_DESC textureDesc = {};
		switch (metadata.dimension)
		{
		case DirectX::TEX_DIMENSION_TEXTURE1D:
			textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT16>(metadata.arraySize));
			break;
		case DirectX::TEX_DIMENSION_TEXTURE2D:
			textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT>(metadata.height), static_cast<UINT16>(metadata.arraySize));
			break;
		case DirectX::TEX_DIMENSION_TEXTURE3D:
			textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT>(metadata.height), static_cast<UINT16>(metadata.depth));
			break;
		default:
			throw std::exception("Invalid texture dimension.");
			break;
		}

		{
			CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
			dxutils::ThrowIfFailed(device->CreateCommittedResource(&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&textureResource)));
		}
		// Update the global state tracker.
		DX12ResourceStateTracker::AddGlobalResourceState(textureResource.Get(), D3D12_RESOURCE_STATE_COMMON);

		texture.SetTextureUsage(textureUsage);
		texture.SetD3D12Resource(textureResource);
		texture.CreateViews();
		texture.SetName(fileName);

		std::vector<D3D12_SUBRESOURCE_DATA> subresources(scratchImage.GetImageCount());
		const DirectX::Image* pImages = scratchImage.GetImages();
		for (size_t i = 0; i < scratchImage.GetImageCount(); ++i)
		{
			auto& subresource = subresources[i];
			subresource.RowPitch = pImages[i].rowPitch;
			subresource.SlicePitch = pImages[i].slicePitch;
			subresource.pData = pImages[i].pixels;
		}

		CopyTextureSubresource(texture, 0, static_cast<uint32_t>(subresources.size()), subresources.data());

		if (subresources.size() < textureResource->GetDesc().MipLevels)
		{
			GenerateMips(texture);
		}

		// Add the texture resource to the texture cache.
		std::lock_guard<std::mutex> lock(s_textureCacheMutex);
		s_textureCache[fileName] = textureResource.Get();
	}
}

void DX12CommandList::ClearTexture(const DX12Texture& texture, const float clearColor[4])
{
	TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_d3d12CommandList->ClearRenderTargetView(texture.GetRenderTargetView(), clearColor, 0, nullptr);

	TrackResource(texture);
}

void DX12CommandList::ClearDepthStencilTexture(const DX12Texture& texture, D3D12_CLEAR_FLAGS clearFlags, float depth, uint8_t stencil)
{
	TransitionBarrier(texture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	m_d3d12CommandList->ClearDepthStencilView(texture.GetDepthStencilView(), clearFlags, depth, stencil, 0, nullptr);

	TrackResource(texture);
}

void DX12CommandList::GenerateMips(DX12Texture& texture)
{
	if (m_d3d12CommandListType == D3D12_COMMAND_LIST_TYPE_COPY)
	{
		if (!m_computeCommandList)
		{
			m_computeCommandList = ((DX12Driver*)DX12Driver::GetInstance())->GetDX12CommandQueue(DX12Driver::CommandQueueType::Compute)->GetCommandList();
		}
		m_computeCommandList->GenerateMips(texture);
		return;
	}

	auto d3d12Resource = texture.GetD3D12Resource();

	// If the texture doesn't have a valid resource, do nothing.
	if (!d3d12Resource) return;
	auto d3d12ResourceDesc = d3d12Resource->GetDesc();

	// If the texture only has a single mip level (level 0)
	// do nothing.
	if (d3d12ResourceDesc.MipLevels == 1) return;
	// Currently, only 2D textures are supported.
	if (d3d12ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D || d3d12ResourceDesc.DepthOrArraySize != 1)
	{
		throw std::exception("Generate Mips only supports 2D Textures.");
	}

	if (DX12Texture::IsUAVCompatibleFormat(d3d12ResourceDesc.Format))
	{
		GenerateMips_UAV(texture);
	}
	else if (DX12Texture::IsBGRFormat(d3d12ResourceDesc.Format))
	{
		GenerateMips_BGR(texture);
	}
	else if (DX12Texture::IsSRGBFormat(d3d12ResourceDesc.Format))
	{
		GenerateMips_sRGB(texture);
	}
	else
	{
		throw std::exception("Unsupported texture format for mipmap generation.");
	}
}

void DX12CommandList::PanoToCubemap(DX12Texture& cubemap, const DX12Texture& pano)
{
	ASSERT(false, "Not implemented.");
}

void DX12CommandList::CopyTextureSubresource(DX12Texture& texture, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData)
{
	auto destinationResource = texture.GetD3D12Resource();
	if (destinationResource)
	{
		// Resource must be in the copy-destination state.
		TransitionBarrier(texture, D3D12_RESOURCE_STATE_COPY_DEST);
		FlushResourceBarriers();

		UINT64 requiredSize = GetRequiredIntermediateSize(destinationResource.Get(), firstSubresource, numSubresources);

		auto device = ((DX12Driver*)DX12Driver::GetInstance())->GetD3D12Device();
		// Create a temporary (intermediate) resource for uploading the subresources
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
		CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resourceDesc(CD3DX12_RESOURCE_DESC::Buffer(requiredSize));
		dxutils::ThrowIfFailed(device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&intermediateResource)
		));

		UpdateSubresources(m_d3d12CommandList.Get(), destinationResource.Get(), intermediateResource.Get(), 0, firstSubresource, numSubresources, subresourceData);

		TrackObject(intermediateResource);
		TrackObject(destinationResource);
	}
}

void DX12CommandList::SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData)
{
	// Constant buffers must be 256-byte aligned.
	auto heapAllococation = m_uploadBuffer->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(heapAllococation.CPU, bufferData, sizeInBytes);

	m_d3d12CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, heapAllococation.GPU);
}

void DX12CommandList::SetGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants)
{
	m_d3d12CommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
}

void DX12CommandList::SetCompute32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants)
{
	m_d3d12CommandList->SetComputeRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
}

void DX12CommandList::SetVertexBuffer(uint32_t slot, const DX12VertexBuffer& vertexBuffer)
{
	TransitionBarrier(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	auto vertexBufferView = vertexBuffer.GetVertexBufferView();

	m_d3d12CommandList->IASetVertexBuffers(slot, 1, &vertexBufferView);

	TrackResource(vertexBuffer);
}

void DX12CommandList::SetDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData)
{
	size_t bufferSize = numVertices * vertexSize;

	auto heapAllocation = m_uploadBuffer->Allocate(bufferSize, vertexSize);
	memcpy(heapAllocation.CPU, vertexBufferData, bufferSize);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = heapAllocation.GPU;
	vertexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
	vertexBufferView.StrideInBytes = static_cast<UINT>(vertexSize);

	m_d3d12CommandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
}

void DX12CommandList::SetDynamicIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData)
{
	constexpr size_t sizeofR16 = 2u;
	constexpr size_t sizeofR32 = 4u;
	size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? sizeofR16 : sizeofR32;
	size_t bufferSize = numIndicies * indexSizeInBytes;

	auto heapAllocation = m_uploadBuffer->Allocate(bufferSize, indexSizeInBytes);
	memcpy(heapAllocation.CPU, indexBufferData, bufferSize);

	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.BufferLocation = heapAllocation.GPU;
	indexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
	indexBufferView.Format = indexFormat;

	m_d3d12CommandList->IASetIndexBuffer(&indexBufferView);
}

void DX12CommandList::SetGraphicsDynamicStructuredBuffer(uint32_t slot, size_t numElements, size_t elementSize, const void* bufferData)
{
	size_t bufferSize = numElements * elementSize;

	auto heapAllocation = m_uploadBuffer->Allocate(bufferSize, elementSize);

	memcpy(heapAllocation.CPU, bufferData, bufferSize);

	m_d3d12CommandList->SetGraphicsRootShaderResourceView(slot, heapAllocation.GPU);
}

void DX12CommandList::SetViewport(const D3D12_VIEWPORT& viewport)
{
	SetViewports({ viewport });
}

void DX12CommandList::SetViewports(const std::vector<D3D12_VIEWPORT>& viewports)
{
	assert(viewports.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	m_d3d12CommandList->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
}

void DX12CommandList::SetScissorRect(const D3D12_RECT& scissorRect)
{
	SetScissorRects({ scissorRect });
}

void DX12CommandList::SetScissorRects(const std::vector<D3D12_RECT>& scissorRects)
{
	assert(scissorRects.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	m_d3d12CommandList->RSSetScissorRects(static_cast<UINT>(scissorRects.size()), scissorRects.data());
}

void DX12CommandList::SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState)
{
	m_d3d12CommandList->SetPipelineState(pipelineState.Get());
	TrackObject(pipelineState);
}

void DX12CommandList::SetGraphicsRootSignature(const DX12RootSignature& rootSignature)
{
	auto d3d12RootSignature = rootSignature.GetRootSignature().Get();
	if (m_rootSignature != d3d12RootSignature)
	{
		m_rootSignature = d3d12RootSignature;

		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
		{
			m_dynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
		}

		m_d3d12CommandList->SetGraphicsRootSignature(m_rootSignature);

		TrackObject(m_rootSignature);
	}
}

void DX12CommandList::SetComputeRootSignature(const DX12RootSignature& rootSignature)
{
	auto d3d12RootSignature = rootSignature.GetRootSignature().Get();
	if (m_rootSignature != d3d12RootSignature)
	{
		m_rootSignature = d3d12RootSignature;

		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
		{
			m_dynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
		}

		m_d3d12CommandList->SetComputeRootSignature(m_rootSignature);

		TrackObject(m_rootSignature);
	}
}

void DX12CommandList::SetShaderResourceView(
	uint32_t rootParameterIndex,
	uint32_t descriptorOffset,
	const DX12Resource& resource,
	D3D12_RESOURCE_STATES stateAfter,
	UINT firstSubresource,
	UINT numSubresources,
	const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
{
	if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
	{
		for (uint32_t i = 0; i < numSubresources; ++i)
		{
			TransitionBarrier(resource, stateAfter, firstSubresource + i);
		}
	}
	else
	{
		TransitionBarrier(resource, stateAfter);
	}

	m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(rootParameterIndex, descriptorOffset, 1, resource.GetShaderResourceView(srv));

	TrackResource(resource);
}

void DX12CommandList::SetUnorderedAccessView(
	uint32_t rootParameterIndex,
	uint32_t descrptorOffset,
	const DX12Resource& resource,
	D3D12_RESOURCE_STATES stateAfter,
	UINT firstSubresource,
	UINT numSubresources,
	const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
{
	if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
	{
		for (uint32_t i = 0; i < numSubresources; ++i)
		{
			TransitionBarrier(resource, stateAfter, firstSubresource + i);
		}
	}
	else
	{
		TransitionBarrier(resource, stateAfter);
	}

	m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(rootParameterIndex, descrptorOffset, 1, resource.GetUnorderedAccessView(uav));

	TrackResource(resource);
}

void DX12CommandList::SetRenderTarget(const DX12RenderTarget& renderTarget)
{
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors;
	renderTargetDescriptors.reserve(AttachmentPoint::NumAttachmentPoints);

	const auto& textures = renderTarget.GetTextures();

	// Bind color targets (max of 8 render targets can be bound to the rendering pipeline.
	for (int i = 0; i < 8; ++i)
	{
		auto& texture = textures[i];

		if (texture.IsValid())
		{
			TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
			renderTargetDescriptors.emplace_back(texture.GetRenderTargetView());

			TrackResource(texture);
		}
	}

	const auto& depthTexture = renderTarget.GetTexture(AttachmentPoint::DepthStencil);

	CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilDescriptor(D3D12_DEFAULT);
	if (depthTexture.GetD3D12Resource())
	{
		TransitionBarrier(depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		depthStencilDescriptor = depthTexture.GetDepthStencilView();

		TrackResource(depthTexture);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE* pDSV = depthStencilDescriptor.ptr != 0 ? &depthStencilDescriptor : nullptr;

	m_d3d12CommandList->OMSetRenderTargets(static_cast<UINT>(renderTargetDescriptors.size()),
		renderTargetDescriptors.data(), FALSE, pDSV);
}

void DX12CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
{
	FlushResourceBarriers();

	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_dynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw(*this);
	}

	m_d3d12CommandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
}

void DX12CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex, int32_t baseVertex, uint32_t startInstance)
{
	FlushResourceBarriers();

	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_dynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw(*this);
	}

	m_d3d12CommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
}

void DX12CommandList::Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
{
	FlushResourceBarriers();

	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_dynamicDescriptorHeap[i]->CommitStagedDescriptorsForDispatch(*this);
	}

	m_d3d12CommandList->Dispatch(numGroupsX, numGroupsY, numGroupsZ);
}

bool DX12CommandList::Close(DX12CommandList& pendingCommandList)
{
	// Flush any remaining barriers.
	FlushResourceBarriers();

	m_d3d12CommandList->Close();

	// Flush pending resource barriers.
	uint32_t numPendingBarriers = m_resourceStateTracker->FlushPendingResourceBarriers(pendingCommandList);
	// Commit the final resource state to the global state.
	m_resourceStateTracker->CommitFinalResourceStates();

	return numPendingBarriers > 0;
}

void DX12CommandList::Close()
{
	FlushResourceBarriers();
	m_d3d12CommandList->Close();
}

void DX12CommandList::Reset()
{
	dxutils::ThrowIfFailed(m_d3d12CommandAllocator->Reset());
	dxutils::ThrowIfFailed(m_d3d12CommandList->Reset(m_d3d12CommandAllocator.Get(), nullptr));

	m_resourceStateTracker->Reset();
	m_uploadBuffer->Reset();

	ReleaseTrackedObjects();

	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_dynamicDescriptorHeap[i]->Reset();
		m_descriptorHeaps[i] = nullptr;
	}

	m_rootSignature = nullptr;
	m_computeCommandList = nullptr;
}

void DX12CommandList::ReleaseTrackedObjects()
{
	m_TrackedObjects.clear();
}

void DX12CommandList::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap)
{
	if (m_descriptorHeaps[heapType] != heap)
	{
		m_descriptorHeaps[heapType] = heap;
		BindDescriptorHeaps();
	}
}

void DX12CommandList::TrackObject(Microsoft::WRL::ComPtr<ID3D12Object> object)
{
	m_TrackedObjects.push_back(object);
}

void DX12CommandList::TrackResource(const DX12Resource& res)
{
	TrackObject(res.GetD3D12Resource());
}

void DX12CommandList::GenerateMips_UAV(DX12Texture& texture)
{
	if (!m_GenerateMipsPSO)
	{
		m_GenerateMipsPSO = std::make_unique<DX12GenerateMipsPSO>();
	}

	auto device = ((DX12Driver*)DX12Driver::GetInstance())->GetD3D12Device();

	auto resource = texture.GetD3D12Resource();
	auto resourceDesc = resource->GetDesc();

	auto stagingResource = resource;
	DX12Texture stagingTexture(stagingResource);
	// If the passed-in resource does not allow for UAV access
	// then create a staging resource that is used to generate
	// the mipmap chain.
	if ((resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
	{
		auto stagingDesc = resourceDesc;
		stagingDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		{
			CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
			dxutils::ThrowIfFailed(device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&stagingDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&stagingResource)
			));
		}

		DX12ResourceStateTracker::AddGlobalResourceState(stagingResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);

		stagingTexture.SetD3D12Resource(stagingResource);
		stagingTexture.CreateViews();
		stagingTexture.SetName(L"Generate Mips UAV Staging Texture");

		CopyResource(stagingTexture, texture);
	}

	m_d3d12CommandList->SetPipelineState(m_GenerateMipsPSO->GetPipelineState().Get());
	SetComputeRootSignature(m_GenerateMipsPSO->GetRootSignature());

	DX12GenerateMipsCB generateMipsCB;

	for (uint32_t srcMip = 0; srcMip < resourceDesc.MipLevels - 1u; )
	{
		uint64_t srcWidth = resourceDesc.Width >> srcMip;
		uint32_t srcHeight = resourceDesc.Height >> srcMip;
		uint32_t dstWidth = static_cast<uint32_t>(srcWidth >> 1);
		uint32_t dstHeight = srcHeight >> 1;

		// Determine the compute shader to use based on the dimension of the 
		// source texture.
		// 0b00(0): Both width and height are even.
		// 0b01(1): Width is odd, height is even.
		// 0b10(2): Width is even, height is odd.
		// 0b11(3): Both width and height are odd.
		generateMipsCB.SrcDimension = (srcHeight & 1) << 1 | (srcWidth & 1);

		// How many mipmap levels to compute this pass (max 4 mips per pass)
		DWORD mipCount;

		// The number of times we can half the size of the texture and get
		// exactly a 50% reduction in size.
		// A 1 bit in the width or height indicates an odd dimension.
		// The case where either the width or the height is exactly 1 is handled
		// as a special case (as the dimension does not require reduction).
		_BitScanForward(&mipCount, (dstWidth == 1 ? dstHeight : dstWidth) |
			(dstHeight == 1 ? dstWidth : dstHeight));
		// Maximum number of mips to generate is 4.
		constexpr DWORD maxMipsNum = 4;
		mipCount = std::min<DWORD>(maxMipsNum, mipCount + 1);
		// Clamp to total number of mips left over.
		mipCount = (srcMip + mipCount) > resourceDesc.MipLevels ?
			resourceDesc.MipLevels - srcMip : mipCount;

		// Dimensions should not reduce to 0.
		// This can happen if the width and height are not the same.
		dstWidth = std::max<DWORD>(1, dstWidth);
		dstHeight = std::max<DWORD>(1, dstHeight);

		generateMipsCB.SrcMipLevel = srcMip;
		generateMipsCB.NumMipLevels = mipCount;
		generateMipsCB.TexelSize.x = 1.0f / (float)dstWidth;
		generateMipsCB.TexelSize.y = 1.0f / (float)dstHeight;

		SetCompute32BitConstants(GenerateMips::GenerateMipsCB, generateMipsCB);

		SetShaderResourceView(GenerateMips::SrcMip, 0, stagingTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, srcMip, 1);
		for (uint32_t mip = 0; mip < mipCount; ++mip)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = resourceDesc.Format;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = srcMip + mip + 1;

			SetUnorderedAccessView(GenerateMips::OutMip, mip, stagingTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, srcMip + mip + 1, 1, &uavDesc);
		}
		// Pad any unused mip levels with a default UAV. Doing this keeps the DX12 runtime happy.
		if (mipCount < maxMipsNum)
		{
			m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(GenerateMips::OutMip, mipCount, maxMipsNum - mipCount, m_GenerateMipsPSO->GetDefaultUAV());
		}

		Dispatch(math::DivideByMultiple(dstWidth, 8), math::DivideByMultiple(dstHeight, 8));

		UAVBarrier(stagingTexture);

		srcMip += mipCount;
	}

	// Copy back to the original texture.
	if (stagingResource != resource)
	{
		CopyResource(texture, stagingTexture);
	}
}

void DX12CommandList::GenerateMips_BGR(DX12Texture& texture)
{
	auto device = ((DX12Driver*)DX12Driver::GetInstance())->GetD3D12Device();

	auto resource = texture.GetD3D12Resource();
	auto resourceDesc = resource->GetDesc();

	// Create a new resource with a UAV compatible format and UAV flags.
	auto copyDesc = resourceDesc;
	copyDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	copyDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	// Create a heap to alias the resource. This is used to copy the resource without 
	// failing GPU validation.
	auto allocationInfo = device->GetResourceAllocationInfo(0, 1, &resourceDesc);
	auto bufferSize = GetRequiredIntermediateSize(resource.Get(), 0, resourceDesc.MipLevels);

	D3D12_HEAP_DESC heapDesc = {};
	heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
	heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D12Heap> heap;
	dxutils::ThrowIfFailed(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));

	Microsoft::WRL::ComPtr<ID3D12Resource> resourceCopy;
	dxutils::ThrowIfFailed(device->CreatePlacedResource(
		heap.Get(),
		0,
		&copyDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&resourceCopy)
	));

	DX12ResourceStateTracker::AddGlobalResourceState(resourceCopy.Get(), D3D12_RESOURCE_STATE_COMMON);

	DX12Texture copyTexture(resourceCopy);

	// Create an alias for which to perform the copy operation.
	auto aliasDesc = resourceDesc;
	aliasDesc.Format = (resourceDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM ||
		resourceDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB) ?
		DXGI_FORMAT_B8G8R8X8_UNORM : DXGI_FORMAT_B8G8R8A8_UNORM;

	Microsoft::WRL::ComPtr<ID3D12Resource> aliasCopy;
	dxutils::ThrowIfFailed(device->CreatePlacedResource(
		heap.Get(),
		0,
		&aliasDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&aliasCopy)
	));

	DX12ResourceStateTracker::AddGlobalResourceState(aliasCopy.Get(), D3D12_RESOURCE_STATE_COMMON);

	// Copy the original texture to the aliased texture.
	DX12Texture aliasTexture(aliasCopy);
	AliasingBarrier(DX12Texture(), aliasTexture); // There is no "before" texture. 
											  // Default constructed Texture is equivalent to a "null" texture.
	CopyResource(aliasTexture, texture);

	// Alias the UAV compatible texture back.
	AliasingBarrier(aliasTexture, copyTexture);
	// Now use the resource copy to generate the mips.
	GenerateMips_UAV(copyTexture);

	// Copy back to the original (via the alias to ensure GPU validation)
	AliasingBarrier(copyTexture, aliasTexture);
	CopyResource(texture, aliasTexture);

	// Track resource to ensure the lifetime.
	TrackObject(heap);
	TrackResource(copyTexture);
	TrackResource(aliasTexture);
	TrackResource(texture);
}

void DX12CommandList::GenerateMips_sRGB(DX12Texture& texture)
{
	auto device = ((DX12Driver*)DX12Driver::GetInstance())->GetD3D12Device();

	// Create a UAV compatible texture.
	auto resource = texture.GetD3D12Resource();
	auto resourceDesc = resource->GetDesc();

	// Create a copy of the original texture with UAV compatible format and UAV flags.
	auto copyDesc = resourceDesc;
	copyDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	copyDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	// Create the resource as a placed resource in a heap to perform an aliased copy.
	// Create a heap to alias the resource. This is used to copy the resource without 
	// failing GPU validation.
	auto allocationInfo = device->GetResourceAllocationInfo(0, 1, &resourceDesc);
	auto bufferSize = GetRequiredIntermediateSize(resource.Get(), 0, resourceDesc.MipLevels);

	D3D12_HEAP_DESC heapDesc = {};
	heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
	heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D12Heap> heap;
	dxutils::ThrowIfFailed(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));

	Microsoft::WRL::ComPtr<ID3D12Resource> resourceCopy;
	dxutils::ThrowIfFailed(device->CreatePlacedResource(
		heap.Get(),
		0,
		&copyDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&resourceCopy)
	));

	DX12ResourceStateTracker::AddGlobalResourceState(resourceCopy.Get(), D3D12_RESOURCE_STATE_COMMON);

	DX12Texture copyTexture(resourceCopy);

	// Create an alias for which to perform the copy operation.
	auto aliasDesc = resourceDesc;

	Microsoft::WRL::ComPtr<ID3D12Resource> aliasCopy;
	dxutils::ThrowIfFailed(device->CreatePlacedResource(
		heap.Get(),
		0,
		&aliasDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&aliasCopy)
	));

	DX12ResourceStateTracker::AddGlobalResourceState(aliasCopy.Get(), D3D12_RESOURCE_STATE_COMMON);

	// Copy the original texture to the aliased texture.
	DX12Texture aliasTexture(aliasCopy);
	AliasingBarrier(DX12Texture(), aliasTexture); // There is no "before" texture. 
											  // Default constructed Texture is equivalent to a "null" texture.
	CopyResource(aliasTexture, texture);

	// Alias the UAV compatible texture back.
	AliasingBarrier(aliasTexture, copyTexture);
	// Now use the resource copy to generate the mips.
	GenerateMips_UAV(copyTexture);

	// Copy back to the original (via the alias to ensure GPU validation)
	AliasingBarrier(copyTexture, aliasTexture);
	CopyResource(texture, aliasTexture);

	// Track resource to ensure the lifetime.
	TrackObject(heap);
	TrackResource(copyTexture);
	TrackResource(aliasTexture);
	TrackResource(texture);
}

void DX12CommandList::CopyBuffer(DX12Buffer& buffer, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
	size_t bufferSize = numElements * elementSize;

	Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource;
	if (bufferSize == 0)
	{
		// This will result in a NULL resource (which may be desired to define a default null resource).
	}
	else
	{
		auto device = ((DX12Driver*)DX12Driver::GetInstance())->GetD3D12Device();
		CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
		dxutils::ThrowIfFailed(device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&d3d12Resource)));

		// Add the resource to the global resource state tracker.
		DX12ResourceStateTracker::AddGlobalResourceState(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON);

		if (bufferData != nullptr)
		{
			// Create an upload resource to use as an intermediate buffer to copy the buffer resource 
			CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
			Microsoft::WRL::ComPtr<ID3D12Resource> uploadResource;
			dxutils::ThrowIfFailed(device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&uploadResource)));

			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = bufferData;
			subresourceData.RowPitch = bufferSize;
			subresourceData.SlicePitch = subresourceData.RowPitch;

			m_resourceStateTracker->TransitionResource(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
			FlushResourceBarriers();

			UpdateSubresources(m_d3d12CommandList.Get(), d3d12Resource.Get(),
				uploadResource.Get(), 0, 0, 1, &subresourceData);

			// Add references to resources so they stay in scope until the command list is reset.
			TrackObject(uploadResource);
		}
		TrackObject(d3d12Resource);
	}

	buffer.SetD3D12Resource(d3d12Resource);
	buffer.CreateViews(numElements, elementSize);
}

void DX12CommandList::BindDescriptorHeaps()
{
	UINT numDescriptorHeaps = 0;
	ID3D12DescriptorHeap* descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

	for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		ID3D12DescriptorHeap* descriptorHeap = m_descriptorHeaps[i];
		if (descriptorHeap)
		{
			descriptorHeaps[numDescriptorHeaps++] = descriptorHeap;
		}
	}

	m_d3d12CommandList->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);
}

}
#endif
}
}