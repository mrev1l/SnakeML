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
  *  @file VertexBuffer.h
  *  @date October 24, 2018
  *  @author Jeremiah van Oosten
  *
  *  @brief Vertex buffer resource.
  */
#pragma endregion

#include "DX12Buffer.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

class DX12VertexBuffer : public DX12Buffer
{
public:
	DX12VertexBuffer(const std::wstring& name = L"");
	virtual ~DX12VertexBuffer() = default;

	// Inherited from Buffer
	virtual void CreateViews(size_t numElements, size_t elementSize) override;

	/**
	 * Get the vertex buffer view for binding to the Input Assembler stage.
	 */
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return m_vertexBufferView; }

	size_t GetNumVertices() const { return m_numVertices; }

	size_t GetVertexStride() const { return m_vertexStride; }

	/**
	* Get the SRV for a resource.
	*/
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;

	/**
	* Get the UAV for a (sub)resource.
	*/
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;

private:
	size_t m_numVertices;
	size_t m_vertexStride;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
};

}
#endif
}
