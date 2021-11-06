#pragma once
#include "lib_includes/directX_includes.h"

namespace snakeml
{

#ifdef _WINDOWS
namespace win
{

class DX12GlobalRenderSettings
{
public:
	DX12GlobalRenderSettings();
	~DX12GlobalRenderSettings() = default;

	CD3DX12_BLEND_DESC GetAlphaBlendDesc()				const { return m_alphaBlendDesc; }
	CD3DX12_DEPTH_STENCIL_DESC GetNoDepthStencilDesc()	const { return m_noDepthStencilDesc; }

private:
	CD3DX12_BLEND_DESC			m_alphaBlendDesc;
	CD3DX12_DEPTH_STENCIL_DESC	m_noDepthStencilDesc;

	static void InitAlphaBlendDesc(CD3DX12_BLEND_DESC& desc);
	static void InitDepthStencilDesc(CD3DX12_DEPTH_STENCIL_DESC& desc);
};

}
#endif

}