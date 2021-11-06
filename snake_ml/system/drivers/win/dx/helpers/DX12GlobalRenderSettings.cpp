// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12GlobalRenderSettings.h"

namespace snakeml
{

#ifdef _WINDOWS
namespace win
{

DX12GlobalRenderSettings::DX12GlobalRenderSettings()
{
	InitAlphaBlendDesc(m_alphaBlendDesc);
	InitDepthStencilDesc(m_noDepthStencilDesc);
}

void DX12GlobalRenderSettings::InitAlphaBlendDesc(CD3DX12_BLEND_DESC& desc)
{
	desc.RenderTarget[0].BlendEnable = TRUE;

	// dest.rgb = src.rgb * src.a + dest.rgb * (1 - src.a)
	desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	// dest.a = 1 - (1 - src.a) * (1 - dest.a) [the math works out]
	desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_DEST_ALPHA;
	desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
	desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

	desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
}

void DX12GlobalRenderSettings::InitDepthStencilDesc(CD3DX12_DEPTH_STENCIL_DESC& desc)
{
	desc = CD3DX12_DEPTH_STENCIL_DESC(
		false,
		D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
		true, 0xFF, 0xFF,
		D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_INCR, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS,
		D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_DECR, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS);
}

}
#endif

}