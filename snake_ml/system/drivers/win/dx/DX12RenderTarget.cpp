// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12RenderTarget.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

DX12RenderTarget::DX12RenderTarget()
	: m_textures(AttachmentPoint::NumAttachmentPoints)
{
}

void DX12RenderTarget::AttachTexture(AttachmentPoint attachmentPoint, const DX12Texture& texture)
{
	m_textures[static_cast<size_t>(attachmentPoint)] = texture;
}

const DX12Texture& DX12RenderTarget::GetTexture(AttachmentPoint attachmentPoint) const
{
	return m_textures[static_cast<size_t>(attachmentPoint)];
}

void DX12RenderTarget::Resize(uint32_t width, uint32_t height)
{
	for (auto& texture : m_textures)
	{
		texture.Resize(width, height);
	}
}

const std::vector<DX12Texture>& DX12RenderTarget::GetTextures() const
{
	return m_textures;
}

D3D12_RT_FORMAT_ARRAY DX12RenderTarget::GetRenderTargetFormats() const
{
	D3D12_RT_FORMAT_ARRAY rtvFormats = {};


	for (int i = AttachmentPoint::Color0; i <= AttachmentPoint::Color7; ++i)
	{
		const DX12Texture& texture = m_textures[i];
		if (texture.IsValid())
		{
			rtvFormats.RTFormats[rtvFormats.NumRenderTargets++] = texture.GetD3D12ResourceDesc().Format;
		}
	}

	return rtvFormats;
}

DXGI_FORMAT DX12RenderTarget::GetDepthStencilFormat() const
{
	DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN;
	const DX12Texture& depthStencilTexture = m_textures[AttachmentPoint::DepthStencil];
	if (depthStencilTexture.IsValid())
	{
		dsvFormat = depthStencilTexture.GetD3D12ResourceDesc().Format;
	}

	return dsvFormat;
}

}
#endif
}
