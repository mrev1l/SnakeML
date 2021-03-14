// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12RootSignature.h"

#include "system/drivers/win/dx/DX12Driver.h"
#include "system/drivers/win/dx/helpers/directX_utils.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

DX12RootSignature::DX12RootSignature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion)
	: m_rootSignatureDesc{}
	, m_numDescriptorsPerTable{ 0 }
	, m_samplerTableBitMask(0)
	, m_descriptorTableBitMask(0)
{
	SetRootSignatureDesc(rootSignatureDesc, rootSignatureVersion);
}

DX12RootSignature::~DX12RootSignature()
{
	Destroy();
}

void DX12RootSignature::Destroy()
{
	for (size_t i = 0; i < static_cast<size_t>(m_rootSignatureDesc.NumParameters); ++i)
	{
		const D3D12_ROOT_PARAMETER1& rootParameter = m_rootSignatureDesc.pParameters[i];
		if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			delete[] rootParameter.DescriptorTable.pDescriptorRanges;
		}
	}

	delete[] m_rootSignatureDesc.pParameters;
	m_rootSignatureDesc.pParameters = nullptr;
	m_rootSignatureDesc.NumParameters = 0;

	delete[] m_rootSignatureDesc.pStaticSamplers;
	m_rootSignatureDesc.pStaticSamplers = nullptr;
	m_rootSignatureDesc.NumStaticSamplers = 0;

	m_descriptorTableBitMask = 0;
	m_samplerTableBitMask = 0;

	memset(m_numDescriptorsPerTable, 0, sizeof(m_numDescriptorsPerTable));
}

uint32_t DX12RootSignature::GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const
{
	uint32_t descriptorTableBitMask = 0;
	switch (descriptorHeapType)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		descriptorTableBitMask = m_descriptorTableBitMask;
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		descriptorTableBitMask = m_samplerTableBitMask;
		break;
	}

	return descriptorTableBitMask;
}

uint32_t DX12RootSignature::GetNumDescriptors(uint32_t rootIndex) const
{
	constexpr uint32_t rootIdxCap = 32u;
	ASSERT(rootIndex < rootIdxCap, "DX12RootSignature::GetNumDescriptors : rootIndex out of range.");
	return m_numDescriptorsPerTable[rootIndex];
}

void DX12RootSignature::SetRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion)
{
	// Make sure any previously allocated root signature description is cleaned 
	// up first.
	Destroy();

	auto device = ((DX12Driver*)DX12Driver::GetInstance())->GetD3D12Device();

	size_t numParameters = static_cast<size_t>(rootSignatureDesc.NumParameters);
	D3D12_ROOT_PARAMETER1* pParameters = numParameters > 0 ? new D3D12_ROOT_PARAMETER1[numParameters] : nullptr;

	for (size_t i = 0; i < numParameters; ++i)
	{
		const D3D12_ROOT_PARAMETER1& rootParameter = rootSignatureDesc.pParameters[i];
		pParameters[i] = rootParameter; //-V522

		if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			size_t numDescriptorRanges = static_cast<size_t>(rootParameter.DescriptorTable.NumDescriptorRanges);
			D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges = numDescriptorRanges > 0 ? new D3D12_DESCRIPTOR_RANGE1[numDescriptorRanges] : nullptr;

			memcpy(pDescriptorRanges, rootParameter.DescriptorTable.pDescriptorRanges, //-V575
				sizeof(D3D12_DESCRIPTOR_RANGE1) * numDescriptorRanges);

			pParameters[i].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(numDescriptorRanges);
			pParameters[i].DescriptorTable.pDescriptorRanges = pDescriptorRanges;

			// Set the bit mask depending on the type of descriptor table.
			if (numDescriptorRanges > 0)
			{
				switch (pDescriptorRanges[0].RangeType)
				{
				case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
				case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
				case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
					m_descriptorTableBitMask |= (1 << i);
					break;
				case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
					m_samplerTableBitMask |= (1 << i);
					break;
				}
			}

			// Count the number of descriptors in the descriptor table.
			for (size_t j = 0; j < numDescriptorRanges; ++j)
			{
				m_numDescriptorsPerTable[i] += pDescriptorRanges[j].NumDescriptors; //-V127
			}
		}
	}

	m_rootSignatureDesc.NumParameters = static_cast<UINT>(numParameters);
	m_rootSignatureDesc.pParameters = pParameters;

	size_t numStaticSamplers = static_cast<size_t>(rootSignatureDesc.NumStaticSamplers);
	D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = numStaticSamplers > 0 ? new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers] : nullptr;

	if (pStaticSamplers)
	{
		memcpy(pStaticSamplers, rootSignatureDesc.pStaticSamplers, sizeof(D3D12_STATIC_SAMPLER_DESC) * numStaticSamplers);
	}

	m_rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(numStaticSamplers);
	m_rootSignatureDesc.pStaticSamplers = pStaticSamplers;

	D3D12_ROOT_SIGNATURE_FLAGS flags = rootSignatureDesc.Flags;
	m_rootSignatureDesc.Flags = flags;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionRootSignatureDesc;
	versionRootSignatureDesc.Init_1_1(static_cast<UINT>(numParameters), pParameters, static_cast<UINT>(numStaticSamplers), pStaticSamplers, flags);

	// Serialize the root signature.
	Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	DX12Utils::ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&versionRootSignatureDesc,
		rootSignatureVersion, &rootSignatureBlob, &errorBlob));

	// Create the root signature.
	DX12Utils::ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

}
#endif
}
}