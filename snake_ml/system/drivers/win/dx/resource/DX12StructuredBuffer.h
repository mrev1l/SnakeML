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
 *  @file StructuredBuffer.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Structured buffer resource.
  */
#pragma endregion

#include "DX12Buffer.h"
#include "DX12ByteAddressBuffer.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

class DX12StructuredBuffer : public DX12Buffer
{
public:
	DX12StructuredBuffer( const std::wstring& name = L"" );
	DX12StructuredBuffer( const D3D12_RESOURCE_DESC& resDesc,
		size_t numElements, size_t elementSize,
		const std::wstring& name = L"");

	/**
	* Get the number of elements contained in this buffer.
	*/
	virtual size_t GetNumElements() const { return m_numElements; }

	/**
	* Get the size in bytes of each element in this buffer.
	*/
	virtual size_t GetElementSize() const { return m_elementSize; }

	/**
	 * Get the SRV for a resource.
	 */
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const
	{
		return m_SRV.GetDescriptorHandle();
	}

	/**
	 * Get the UAV for a (sub)resource.
	 */
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override
	{
		// Buffers don't have subresources.
		return m_UAV.GetDescriptorHandle();
	}

	const DX12ByteAddressBuffer& GetCounterBuffer() const { return m_counterBuffer; }

	/**
	 * Create the views for the buffer resource.
	 * Used by the CommandList when setting the buffer contents.
	 */
	virtual void CreateViews(size_t numElements, size_t elementSize) override;

private:
	size_t m_numElements = 0u;
	size_t m_elementSize = 0u;

	DX12DescriptorAllocation m_SRV;
	DX12DescriptorAllocation m_UAV;

	// A buffer to store the internal counter for the structured buffer.
	DX12ByteAddressBuffer m_counterBuffer;
};

}
#endif
}
}