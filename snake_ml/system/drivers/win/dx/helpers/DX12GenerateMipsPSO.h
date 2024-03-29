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
  *  @file GenerateMipsPSO.h
  *  @date October 24, 2018
  *  @author Jeremiah van Oosten
  *
  *  @brief Pipeline state object for generating mip maps.
  */
#pragma endregion

#include "system/drivers/win/dx/pipeline/DX12RootSignature.h"
#include "system/drivers/win/dx/resource_management/DX12DescriptorAllocation.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

struct alignas( 16 ) DX12GenerateMipsCB
{
	uint32_t SrcMipLevel;	// Texture level of source mip
	uint32_t NumMipLevels;	// Number of OutMips to write: [1-4]
	uint32_t SrcDimension;  // Width and height of the source texture are even or odd.
	uint32_t Padding;	   // Pad to 16 byte alignment.
	DirectX::XMFLOAT2 TexelSize;	// 1.0 / OutMip1.Dimensions
};

// I don't use scoped enums to avoid the explicit cast that is required to 
// treat these as root indices.
namespace GenerateMips
{
	enum
	{
		GenerateMipsCB,
		SrcMip,
		OutMip,
		NumRootParameters
	};
}

class DX12GenerateMipsPSO
{
public:
	DX12GenerateMipsPSO();
	~DX12GenerateMipsPSO() = default;

	const DX12RootSignature& GetRootSignature() const { return m_rootSignature; }
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState() const { return m_pipelineState; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultUAV() const { return m_defaultUAV.GetDescriptorHandle(); }

private:
	DX12RootSignature m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	// Default (no resource) UAV's to pad the unused UAV descriptors.
	// If generating less than 4 mip map levels, the unused mip maps
	// need to be padded with default UAVs (to keep the DX12 runtime happy).
	DX12DescriptorAllocation m_defaultUAV;
};

}
#endif
}