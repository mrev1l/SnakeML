// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12CommandList.h"

#include "DX12Buffer.h"
#include "DX12Driver.h"
#include "DX12DynamicDescriptorHeap.h"
#include "DX12Resource.h"
#include "DX12ResourceStateTracker.h"
#include "DX12RootSignature.h"
#include "DX12Texture.h"
#include "DX12VertexBuffer.h"
#include "DX12UploadBuffer.h"

namespace snakeml
{
namespace system
{
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
    ASSERT(false, "Not implemented!");
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
        ASSERT(false, "Not implemented.");
        //GenerateMips_UAV(texture);
    }
    else if (DX12Texture::IsBGRFormat(d3d12ResourceDesc.Format))
    {
        ASSERT(false, "Not implemented.");
        //GenerateMips_BGR(texture);
    }
    else if (DX12Texture::IsSRGBFormat(d3d12ResourceDesc.Format))
    {
        ASSERT(false, "Not implemented.");
        //GenerateMips_sRGB(texture);
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
}
}