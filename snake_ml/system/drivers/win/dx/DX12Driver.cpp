// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12Driver.h"

#include "DX12ResourceStateTracker.h"
#include "DX12TextureUsage.h"
#include "system/drivers/win/os/WinDriver.h"
#include "utils/win_utils.h"

namespace snakeml
{
namespace system
{
namespace win
{

DX12Driver::DX12Driver(HWND windowHandle, math::vec2<uint32_t> windowSz)
	: IRenderDriver()
	, m_osWindowHandle(windowHandle)
	, m_clientWidth(static_cast<FLOAT>(windowSz.m_x))
	, m_clientHeight(static_cast<FLOAT>(windowSz.m_y))
	, m_viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, windowSz.m_x, windowSz.m_y))
	, m_scissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
{
	for (int i = 0; i < s_backBufferCount; ++i)
	{
		m_backBufferTextures[i].SetName(L"Backbuffer[" + std::to_wstring(i) + L"]");
	}
}

DX12Driver::~DX12Driver()
{
	Flush();
}

std::shared_ptr<DX12CommandQueue> DX12Driver::GetDX12CommandQueue(CommandQueueType type)
{
	switch (type)
	{
	case CommandQueueType::Direct: return m_directCommandQueue;
	case CommandQueueType::Compute: return m_computeCommandQueue;
	case CommandQueueType::Copy: return m_copyCommandQueue;
	}
	ASSERT(false, "Unknown command queue type");
	return nullptr;
}

DX12DescriptorAllocation DX12Driver::AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
{
	return m_descriptorAllocators[type]->Allocate(numDescriptors);
}

void DX12Driver::ReleaseStaleDescriptors(uint64_t finishedFrame)
{
	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_descriptorAllocators[i]->ReleaseStaleDescriptors(finishedFrame);
	}
}

DXGI_SAMPLE_DESC DX12Driver::GetMultisampleQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags) const
{
	DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels;
	qualityLevels.Format = format;
	qualityLevels.SampleCount = 1;
	qualityLevels.Flags = flags;
	qualityLevels.NumQualityLevels = 0;

	while (qualityLevels.SampleCount <= numSamples && SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))) && qualityLevels.NumQualityLevels > 0)
	{
		// That works...
		sampleDesc.Count = qualityLevels.SampleCount;
		sampleDesc.Quality = qualityLevels.NumQualityLevels - 1;

		// But can we do better?
		qualityLevels.SampleCount *= 2;
	}

	return sampleDesc;
}

void DX12Driver::OnInitialize()
{
	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window 
	// to achieve 100% scaling while still allowing non-client window content to 
	// be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	dxutils::EnableDebugLayer();

	m_isTearingSupported = dxutils::CheckTearingSupport();
	// Check for DirectX Math library support.
	if (!DirectX::XMVerifyCPUSupport())
	{
		MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter4 = dxutils::GetAdapter(m_isUsingWarp);
	m_device = dxutils::CreateDevice(dxgiAdapter4);

	m_directCommandQueue = std::make_shared<DX12CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_computeCommandQueue = std::make_shared<DX12CommandQueue>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	m_copyCommandQueue = std::make_shared<DX12CommandQueue>(D3D12_COMMAND_LIST_TYPE_COPY);
	
	m_swapChain = dxutils::CreateSwapChain(m_osWindowHandle, m_directCommandQueue->GetD3D12CommandQueue(), 
		static_cast<uint32_t>(m_clientWidth), static_cast<uint32_t>(m_clientHeight), s_backBufferCount);
	
	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	InitializeDescriptorAllocators();
	InitializeBackBufferTextures();
	InitializeRenderTarget();
	InitializeMatrices();
}

void DX12Driver::OnShutdown()
{
	Flush();
}

void DX12Driver::OnRender()
{
}

void DX12Driver::Flush()
{
	m_directCommandQueue->Flush();
	m_computeCommandQueue->Flush();
	m_copyCommandQueue->Flush();
}

void DX12Driver::InitializeDescriptorAllocators()
{
	for (unsigned int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_descriptorAllocators[i] = std::make_unique<DX12DescriptorAllocator>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
	}
}

void DX12Driver::InitializeBackBufferTextures()
{
	for (uint8_t i = 0; i < s_backBufferCount; ++i)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
		dxutils::ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

		DX12ResourceStateTracker::AddGlobalResourceState(backBuffer.Get(), D3D12_RESOURCE_STATE_COMMON);

		m_backBufferTextures[i].SetD3D12Resource(backBuffer);
		m_backBufferTextures[i].CreateViews();
	}
}

void DX12Driver::InitializeRenderTarget()
{
	// sRGB formats provide free gamma correction!
	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// Check the best multisample quality level that can be used for the given back buffer format.
	DXGI_SAMPLE_DESC sampleDesc = GetMultisampleQualityLevels(backBufferFormat, D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT);
	
	D3D12_CLEAR_VALUE depthClearValue;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthClearValue.DepthStencil = { 1.0f, 0 };

	InitializeRenderTarget_AttachTexture(
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		sampleDesc,
		D3D12_CLEAR_VALUE{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, s_defaultClearColor[0], s_defaultClearColor[1], s_defaultClearColor[2], s_defaultClearColor[3] },
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		TextureUsage::RenderTarget,
		AttachmentPoint::Color0,
		L"Color Render Target"
	);

	InitializeRenderTarget_AttachTexture(
		DXGI_FORMAT_D32_FLOAT,
		sampleDesc,
		depthClearValue,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		TextureUsage::Depth,
		AttachmentPoint::DepthStencil,
		L"Depth Render Target"
	);
}

void DX12Driver::InitializeRenderTarget_AttachTexture(DXGI_FORMAT format, const DXGI_SAMPLE_DESC& sampleDesc, const D3D12_CLEAR_VALUE& clearValue,
	D3D12_RESOURCE_FLAGS resourceFlags, TextureUsage usage, AttachmentPoint attachmentPoint, const std::wstring& name)
{
	auto colorDesc = CD3DX12_RESOURCE_DESC::Tex2D(format,
		m_clientWidth, m_clientHeight,
		1, 1,
		sampleDesc.Count, sampleDesc.Quality,
		resourceFlags);

	DX12Texture texture = DX12Texture(colorDesc, &clearValue, usage, name);

	m_renderTarget.AttachTexture(attachmentPoint, texture);
}

void DX12Driver::InitializeMatrices()
{
	m_orthogonalMatrix = DirectX::XMMatrixOrthographicLH(m_clientWidth, m_clientHeight, 0.1f, 100.f);

	const DirectX::XMVECTOR eyePosition = DirectX::XMVectorSet(0, 0, -10, 1);
	const DirectX::XMVECTOR focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
	const DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
	m_viewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

	float aspectRatio = static_cast<float>(m_clientWidth) / static_cast<float>(m_clientHeight);
	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_foV), aspectRatio, 0.1f, 100.0f);
}



}
}
}
