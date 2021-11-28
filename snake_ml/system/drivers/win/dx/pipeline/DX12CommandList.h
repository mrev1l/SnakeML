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
  *  @file CommandList.h
  *  @date October 22, 2018
  *  @author Jeremiah van Oosten
  *
  *  @brief CommandList class encapsulates a ID3D12GraphicsCommandList2 interface.
  *  The CommandList class provides additional functionality that makes working with
  *  DirectX 12 applications easier.
  */
#pragma endregion

#include "lib_includes/directX_includes.h"

#include "system/drivers/win/dx/resource/DX12TextureUsage.h"

#include "third_party/win/DirectXTex/DirectXTex/DirectXTex.h"

#include <filesystem>
#include <map>
#include <mutex>

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

class DX12ByteAddressBuffer;
class DX12Buffer;
class DX12DynamicDescriptorHeap;
class DX12GenerateMipsPSO;
class DX12IndexBuffer;
class DX12PanoToCubemapPSO;
class DX12RenderTarget;
class DX12Resource;
class DX12ResourceStateTracker;
class DX12RootSignature;
class DX12StructuredBuffer;
class DX12Texture;
class DX12VertexBuffer;
class DX12UploadBuffer;

class DX12CommandList
{
public:
	DX12CommandList(D3D12_COMMAND_LIST_TYPE type);
	virtual ~DX12CommandList();

	/**
	 * Get the type of command list.
	 */
	D3D12_COMMAND_LIST_TYPE GetCommandListType() const { return m_d3d12CommandListType; }

	/**
	 * Get direct access to the ID3D12GraphicsCommandList2 interface.
	 */
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetGraphicsCommandList() const { return m_d3d12CommandList; }

	std::shared_ptr<DX12CommandList> GetGenerateMipsCommandList() const { return m_computeCommandList; }

	/**
	 * Transition a resource to a particular state.
	 *
	 * @param resource The resource to transition.
	 * @param stateAfter The state to transition the resource to. The before state is resolved by the resource state tracker.
	 * @param subresource The subresource to transition. By default, this is D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES which indicates that all subresources are transitioned to the same state.
	 * @param flushBarriers Force flush any barriers. Resource barriers need to be flushed before a command (draw, dispatch, or copy) that expects the resource to be in a particular state can run.
	 */
	void TransitionBarrier(const DX12Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);

	/**
	 * Add a UAV barrier to ensure that any writes to a resource have completed
	 * before reading from the resource.
	 *
	 * @param resource The resource to add a UAV barrier for.
	 * @param flushBarriers Force flush any barriers. Resource barriers need to be
	 * flushed before a command (draw, dispatch, or copy) that expects the resource
	 * to be in a particular state can run.
	 */
	void UAVBarrier(const DX12Resource& resource, bool flushBarriers = false);

	/**
	 * Add an aliasing barrier to indicate a transition between usages of two
	 * different resources that occupy the same space in a heap.
	 *
	 * @param beforeResource The resource that currently occupies the heap.
	 * @param afterResource The resource that will occupy the space in the heap.
	 */
	void AliasingBarrier(const DX12Resource& beforeResource, const DX12Resource& afterResource, bool flushBarriers = false);

	/**
	 * Flush any barriers that have been pushed to the command list.
	 */
	void FlushResourceBarriers();

	/**
	 * Copy resources.
	 */
	void CopyResource(DX12Resource& dstRes, const DX12Resource& srcRes);

	/**
	 * Resolve a multisampled resource into a non-multisampled resource.
	 */
	void ResolveSubresource(DX12Resource& dstRes, const DX12Resource& srcRes, uint32_t dstSubresource = 0, uint32_t srcSubresource = 0);

	/**
	 * Copy the contents to a vertex buffer in GPU memory.
	 */
	void CopyVertexBuffer(DX12VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData);
	template<typename T>
	void CopyVertexBuffer(DX12VertexBuffer& vertexBuffer, const std::vector<T>& vertexBufferData)
	{
		CopyVertexBuffer(vertexBuffer, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
	}

	/**
	 * Copy the contents to a index buffer in GPU memory.
	 */
	void CopyIndexBuffer(DX12IndexBuffer& indexBuffer, size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData);
	template<typename T>
	void CopyIndexBuffer(DX12IndexBuffer& indexBuffer, const std::vector<T>& indexBufferData)
	{
		constexpr size_t indexSize2byte = 2u, indexSize4byte = 4u;
		assert(sizeof(T) == indexSize2byte || sizeof(T) == indexSize4byte);
	
		DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		CopyIndexBuffer(indexBuffer, indexBufferData.size(), indexFormat, indexBufferData.data());
	}

	/**
	 * Copy the contents to a byte address buffer in GPU memory.
	 */
	void CopyByteAddressBuffer(DX12ByteAddressBuffer& byteAddressBuffer, size_t bufferSize, const void* bufferData);
	template<typename T>
	void CopyByteAddressBuffer(DX12ByteAddressBuffer& byteAddressBuffer, const T& data)
	{
		CopyByteAddressBuffer(byteAddressBuffer, sizeof(T), &data);
	}

	/**
	 * Copy the contents to a structured buffer in GPU memory.
	 */
	void CopyStructuredBuffer(DX12StructuredBuffer& structuredBuffer, size_t numElements, size_t elementSize, const void* bufferData);
	template<typename T>
	void CopyStructuredBuffer(DX12StructuredBuffer& structuredBuffer, const std::vector<T>& bufferData)
	{
		CopyStructuredBuffer(structuredBuffer, bufferData.size(), sizeof(T), bufferData.data());
	}

	/**
	 * Set the current primitive topology for the rendering pipeline.
	 */
	void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology);

	/**
	 * Load a texture by a filename.
	 */
	void LoadTexture(DX12Texture& texture, const std::vector<std::wstring>& filePaths, TextureUsage textureUsage = TextureUsage::Albedo);
	
	/**
	 * Clear a texture.
	 */
	void ClearTexture(const DX12Texture& texture, const float clearColor[4]);
	
	/**
	 * Clear depth/stencil texture.
	 */
	void ClearDepthStencilTexture(const DX12Texture& texture, D3D12_CLEAR_FLAGS clearFlags, float depth = 1.0f, uint8_t stencil = 0);
	
	/**
	 * Generate mips for the texture.
	 * The first subresource is used to generate the mip chain.
	 * Mips are automatically generated for textures loaded from files.
	 */
	void GenerateMips(DX12Texture& texture);
	
	/**
	 * Generate a cubemap texture from a panoramic (equirectangular) texture.
	 */
	void PanoToCubemap(DX12Texture& cubemap, const DX12Texture& pano);
	
	/**
	 * Copy subresource data to a texture.
	 */
	void CopyTextureSubresource(DX12Texture& texture, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData);

	/**
	 * Set a dynamic constant buffer data to an inline descriptor in the root
	 * signature.
	 */
	void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData);
	template<typename T>
	void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, const T& data)
	{
		SetGraphicsDynamicConstantBuffer(rootParameterIndex, sizeof(T), &data);
	}

	/**
	 * Set a set of 32-bit constants on the graphics pipeline.
	 */
	void SetGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants);
	template<typename T>
	void SetGraphics32BitConstants(uint32_t rootParameterIndex, const T& constants)
	{
		static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Size of type must be a multiple of 4 bytes");
		SetGraphics32BitConstants(rootParameterIndex, sizeof(T) / sizeof(uint32_t), &constants);
	}

	/**
	 * Set a set of 32-bit constants on the compute pipeline.
	 */
	void SetCompute32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants);
	template<typename T>
	void SetCompute32BitConstants(uint32_t rootParameterIndex, const T& constants)
	{
		static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Size of type must be a multiple of 4 bytes");
		SetCompute32BitConstants(rootParameterIndex, sizeof(T) / sizeof(uint32_t), &constants);
	}

	/**
	 * Set the vertex buffer to the rendering pipeline.
	 */
	void SetVertexBuffer(uint32_t slot, const DX12VertexBuffer& vertexBuffer);

	/**
	 * Set dynamic vertex buffer data to the rendering pipeline.
	 */
	void SetDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData);
	template<typename T>
	void SetDynamicVertexBuffer(uint32_t slot, const std::vector<T>& vertexBufferData)
	{
		SetDynamicVertexBuffer(slot, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
	}

	/**
	 * Bind the index buffer to the rendering pipeline.
	 */
	void SetIndexBuffer(const DX12IndexBuffer& indexBuffer);

	/**
	 * Bind dynamic index buffer data to the rendering pipeline.
	 */
	void SetDynamicIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData);
	template<typename T>
	void SetDynamicIndexBuffer(const std::vector<T>& indexBufferData)
	{
		constexpr size_t sizeofR16 = 2u;
		constexpr size_t sizeofR32 = 4u;
		static_assert(sizeof(T) == sizeofR16 || sizeof(T) == sizeofR32);

		DXGI_FORMAT indexFormat = (sizeof(T) == sizeofR16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		SetDynamicIndexBuffer(indexBufferData.size(), indexFormat, indexBufferData.data());
	}

	/**
	 * Set dynamic structured buffer contents.
	 */
	void SetGraphicsDynamicStructuredBuffer(uint32_t slot, size_t numElements, size_t elementSize, const void* bufferData);
	template<typename T>
	void SetGraphicsDynamicStructuredBuffer(uint32_t slot, const std::vector<T>& bufferData)
	{
		SetGraphicsDynamicStructuredBuffer(slot, bufferData.size(), sizeof(T), bufferData.data());
	}

	/**
	 * Set viewports.
	 */
	void SetViewport(const D3D12_VIEWPORT& viewport);
	void SetViewports(const std::vector<D3D12_VIEWPORT>& viewports);

	/**
	 * Set scissor rects.
	 */
	void SetScissorRect(const D3D12_RECT& scissorRect);
	void SetScissorRects(const std::vector<D3D12_RECT>& scissorRects);

	/**
	 * Set the pipeline state object on the command list.
	 */
	void SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState);

	/**
	 * Set the current root signature on the command list.
	 */
	void SetGraphicsRootSignature(const DX12RootSignature& rootSignature);
	void SetComputeRootSignature(const DX12RootSignature& rootSignature);

	/**
	 * Set the SRV on the graphics pipeline.
	 */
	void SetShaderResourceView(
		uint32_t rootParameterIndex,
		uint32_t descriptorOffset,
		const DX12Resource& resource,
		D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		UINT firstSubresource = 0,
		UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr
	);

	/**
	 * Set the UAV on the graphics pipeline.
	 */
	void SetUnorderedAccessView(
		uint32_t rootParameterIndex,
		uint32_t descrptorOffset,
		const DX12Resource& resource,
		D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		UINT firstSubresource = 0,
		UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr
	);

	/**
	 * Set the render targets for the graphics rendering pipeline.
	 */
	void SetRenderTarget(const DX12RenderTarget& renderTarget);

	/**
	 * Draw geometry.
	 */
	void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0);
	void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0, uint32_t startInstance = 0);

	/**
	 * Dispatch a compute shader.
	 */
	void Dispatch(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1);

	/***************************************************************************
	 * Methods defined below are only intended to be used by internal classes. *
	 ***************************************************************************/

	 /**
	  * Close the command list.
	  * Used by the command queue.
	  *
	  * @param pendingCommandList The command list that is used to execute pending
	  * resource barriers (if any) for this command list.
	  *
	  * @return true if there are any pending resource barriers that need to be
	  * processed.
	  */
	bool Close(DX12CommandList& pendingCommandList);
	// Just close the command list. This is useful for pending command lists.
	void Close();

	/**
	 * Reset the command list. This should only be called by the CommandQueue
	 * before the command list is returned from CommandQueue::GetCommandList.
	 */
	void Reset();

	/**
	 * Release tracked objects. Useful if the swap chain needs to be resized.
	 */
	void ReleaseTrackedObjects();

	/**
	 * Set the currently bound descriptor heap.
	 * Should only be called by the DynamicDescriptorHeap class.
	 */
	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap);

private:
	using TrackedObjects = std::vector<Microsoft::WRL::ComPtr<ID3D12Object>>;

	struct TextureMetadata
	{
		DirectX::TEX_DIMENSION dimension;
		DXGI_FORMAT format;
		UINT64 width;
		UINT64 height;
		UINT16 arraySize;
		UINT16 depth;
	};

	void TrackObject(Microsoft::WRL::ComPtr<ID3D12Object> object);
	void TrackResource(const DX12Resource& res);

	// Generate mips for UAV compatible textures.
	void GenerateMips_UAV(DX12Texture& texture);
	// Generate mips for BGR textures.
	void GenerateMips_BGR(DX12Texture& texture);
	// Generate mips for sRGB textures.
	void GenerateMips_sRGB(DX12Texture& texture);

	// Copy the contents of a CPU buffer to a GPU buffer (possibly replacing the previous buffer contents).
	void CopyBuffer(DX12Buffer& buffer, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	// Binds the current descriptor heaps to the command list.
	void BindDescriptorHeaps();

	static std::vector<std::filesystem::path> LoadTexture_ValidatePaths(const std::vector<std::wstring>& filePaths);
	static bool LoadTexture_InitFromCache(DX12Texture& texture, const std::vector<std::wstring>& filePaths, TextureUsage textureUsage);
	static std::wstring LoadTexture_GenerateTextureArrayName(const std::vector<std::wstring>& filePaths);
	static void LoadTexture_LoadSubresourceData(
		std::vector<std::filesystem::path> texturePaths,
		TextureUsage textureUsage,
		std::vector<DirectX::TexMetadata>& _outFileMetadatas,
		std::vector<DirectX::ScratchImage>& _outScratchImages,
		std::vector<D3D12_SUBRESOURCE_DATA>& _outSubresourceData,
		TextureMetadata& _outTexMetadata);
	static void LoadTexture_LoadTextureFile(const std::filesystem::path& path, DirectX::TexMetadata& _outFileMetadata, DirectX::ScratchImage& _outScratchImage);
	static D3D12_RESOURCE_DESC LoadTexture_CreateResourceDesc(const TextureMetadata& textureMetadata);
	static Microsoft::WRL::ComPtr<ID3D12Resource> LoadTexture_CreateCommitedResource(const D3D12_RESOURCE_DESC& textureDesc);


	D3D12_COMMAND_LIST_TYPE m_d3d12CommandListType;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_d3d12CommandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_d3d12CommandAllocator;

	// For copy queues, it may be necessary to generate mips while loading textures.
	// Mips can't be generated on copy queues but must be generated on compute or
	// direct queues. In this case, a Compute command list is generated and executed 
	// after the copy queue is finished uploading the first sub resource.
	std::shared_ptr<DX12CommandList> m_computeCommandList;

	// Keep track of the currently bound root signatures to minimize root
	// signature changes.
	ID3D12RootSignature* m_rootSignature = nullptr;

	// Resource created in an upload heap. Useful for drawing of dynamic geometry
	// or for uploading constant buffer data that changes every draw call.
	std::unique_ptr<DX12UploadBuffer> m_uploadBuffer;

	// Resource state tracker is used by the command list to track (per command list)
	// the current state of a resource. The resource state tracker also tracks the 
	// global state of a resource in order to minimize resource state transitions.
	std::unique_ptr<DX12ResourceStateTracker> m_resourceStateTracker;

	// The dynamic descriptor heap allows for descriptors to be staged before
	// being committed to the command list. Dynamic descriptors need to be
	// committed before a Draw or Dispatch.
	std::unique_ptr<DX12DynamicDescriptorHeap> m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	// Keep track of the currently bound descriptor heaps. Only change descriptor 
	// heaps if they are different than the currently bound descriptor heaps.
	ID3D12DescriptorHeap* m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	// Pipeline state object for Mip map generation.
	std::unique_ptr<DX12GenerateMipsPSO> m_GenerateMipsPSO;
	
	// Pipeline state object for converting panorama (equirectangular) to cubemaps
	std::unique_ptr<DX12PanoToCubemapPSO> m_PanoToCubemapPSO;

	// Objects that are being tracked by a command list that is "in-flight" on 
	// the command-queue and cannot be deleted. To ensure objects are not deleted 
	// until the command list is finished executing, a reference to the object
	// is stored. The referenced objects are released when the command list is 
	// reset.
	TrackedObjects m_TrackedObjects;

	// Keep track of loaded textures to avoid loading the same texture multiple times.
	static std::map<std::wstring, ID3D12Resource* > s_textureCache;
	static std::mutex s_textureCacheMutex;
};

}
#endif
}