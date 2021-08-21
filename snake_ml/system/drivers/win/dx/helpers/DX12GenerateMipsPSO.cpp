// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12GenerateMipsPSO.h"

#include "system/drivers/win/dx/DX12Driver.h"
#include "system/drivers/win/dx/helpers/directX_utils.h"

#include "system/drivers/win/os/helpers/win_utils.h"

#include "data/shaders/GenerateMips_CS.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

DX12GenerateMipsPSO::DX12GenerateMipsPSO()
{
	DX12Driver* dx12Driver = ((DX12Driver*)DX12Driver::GetInstance());
	auto device = dx12Driver->GetD3D12Device();

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if ( FAILED( device->CheckFeatureSupport( D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof( featureData ) ) ) )
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	constexpr UINT mipsNum = 4;
	CD3DX12_DESCRIPTOR_RANGE1 srcMip( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE );
	CD3DX12_DESCRIPTOR_RANGE1 outMip( D3D12_DESCRIPTOR_RANGE_TYPE_UAV, mipsNum, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE );

	CD3DX12_ROOT_PARAMETER1 rootParameters[GenerateMips::NumRootParameters] = {};
	rootParameters[GenerateMips::GenerateMipsCB].InitAsConstants(sizeof(DX12GenerateMipsCB) / sizeof(DX12GenerateMipsCB::Padding), 0 );
	rootParameters[GenerateMips::SrcMip].InitAsDescriptorTable( 1, &srcMip );
	rootParameters[GenerateMips::OutMip].InitAsDescriptorTable( 1, &outMip );

	CD3DX12_STATIC_SAMPLER_DESC linearClampSampler( 0,
													D3D12_FILTER_MIN_MAG_MIP_LINEAR,
													D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
													D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
													D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc( GenerateMips::NumRootParameters,
															 rootParameters, 1, &linearClampSampler );

	m_rootSignature.SetRootSignatureDesc( rootSignatureDesc.Desc_1_1, featureData.HighestVersion );

	// Create the PSO for GenerateMips shader.
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_CS CS;
	} pipelineStateStream;

	pipelineStateStream.pRootSignature = m_rootSignature.GetRootSignature().Get();
	pipelineStateStream.CS = { g_GenerateMips_CS, sizeof( g_GenerateMips_CS ) };

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof( PipelineStateStream ), &pipelineStateStream
	};

	WinUtils::ThrowIfFailed( device->CreatePipelineState( &pipelineStateStreamDesc, IID_PPV_ARGS( &m_pipelineState ) ) );

	// Create some default texture UAV's to pad any unused UAV's during mip map generation.
	m_defaultUAV = dx12Driver->AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, mipsNum);
	UINT descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

	for ( UINT i = 0; i < mipsNum; ++i )
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		uavDesc.Texture2D.MipSlice = i;
		uavDesc.Texture2D.PlaneSlice = 0;

		device->CreateUnorderedAccessView( nullptr, nullptr, &uavDesc, m_defaultUAV.GetDescriptorHandle(i) );
	}
}

}
#endif
}
