// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "DX12RenderTarget.h"

#include "helpers/DX12DebugRenderingSettings.h"
#include "helpers/XInputHandler.h"

#include "pipeline/DX12CommandQueue.h"
#include "resource_management/DX12DescriptorAllocator.h"

#include "system/drivers/RenderDriver.h"

namespace snakeml
{
class Entity;

#ifdef _WINDOWS
namespace win
{

class DX12Driver : public IRenderDriver
{
	static constexpr std::chrono::milliseconds s_DX12FenceDefaultWait = std::chrono::milliseconds::max();
	static constexpr uint8_t s_backBufferCount = 2u;
	static constexpr FLOAT s_defaultClearColor[4] = { 0.01f,0.01f,0.01f,1.f };

public:
	enum class CommandQueueType
	{
		Direct,
		Compute,
		Copy
	};

	DX12Driver(HWND windowHandle, uint32_t2 windowSz);
	~DX12Driver();

	void SubscribeForRendering(const Entity& renderable);

	Microsoft::WRL::ComPtr<ID3D12Device2> GetD3D12Device() { return m_device; }
	std::shared_ptr<DX12CommandQueue> GetDX12CommandQueue(CommandQueueType type);

	/**
	 * Allocate a number of CPU visible descriptors.
	 */
	DX12DescriptorAllocation AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors = 1);

	/**
	 * Release stale descriptors. This should only be called with a completed frame counter.
	 */
	void ReleaseStaleDescriptors(uint64_t finishedFrame);

	/**
	 * Check if the requested multisample quality is supported for the given format.
	 */
	DXGI_SAMPLE_DESC GetMultisampleQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE) const;

	void GetMatrices(matrix& outProjection, matrix& outOrthogonal) const;

	const DX12DebugRenderingSettings& GetDebugRenderingSettings() const { return m_debugRenderingSettings; }
	XInputHandler& GetXInputHandler() { return m_xInputHandler; }

private:
	void OnInitialize() override;
	void OnShutdown() override;

	// ******* Frame Rendering *******
	void OnRender() override;
	// Begin Frame
	void OnRender_BeginFrame(std::shared_ptr<DX12CommandList> commandList);
	void OnRender_BeginFrame_ClearRenderTargets(std::shared_ptr<DX12CommandList> commandList);
	void OnRender_BeginFrame_SetupRenderTargets(std::shared_ptr<DX12CommandList> commandList);
	// Execute Frame
	void OnRender_ExecuteFrame(std::shared_ptr<DX12CommandQueue> commandQueue, std::shared_ptr<DX12CommandList> commandList);
	// End Frame
	void OnRender_EndFrame(std::shared_ptr<DX12CommandQueue> commandQueue, std::shared_ptr<DX12CommandList> commandList);
	void OnRender_EndFrame_Present(std::shared_ptr<DX12CommandList> commandList, std::shared_ptr<DX12CommandQueue> commandQueue);
	// ******* ******* ******* *******

	void Flush();

	void InitializeDescriptorAllocators();

	void InitializeBackBufferTextures();

	void InitializeRenderTarget();
	void InitializeRenderTarget_AttachTexture(DXGI_FORMAT format, const DXGI_SAMPLE_DESC& sampleDesc, const D3D12_CLEAR_VALUE& clearValue, 
		D3D12_RESOURCE_FLAGS resourceFlags, TextureUsage usage, AttachmentPoint attachmentPoint, const std::wstring& name);

	void InitializeMatrices();

	// DirectX 12 Objects
	Microsoft::WRL::ComPtr<ID3D12Device2> m_device;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
	
	std::shared_ptr<DX12CommandQueue> m_directCommandQueue;
	std::shared_ptr<DX12CommandQueue> m_computeCommandQueue;
	std::shared_ptr<DX12CommandQueue> m_copyCommandQueue;

	std::unique_ptr<DX12DescriptorAllocator> m_descriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	DX12RenderTarget m_renderTarget;
	DX12Texture m_backBufferTextures[s_backBufferCount];
	UINT m_currentBackBufferIndex = 0;

	DX12DebugRenderingSettings m_debugRenderingSettings;
	XInputHandler m_xInputHandler;

	// Synchronization objects
	std::array<uint64_t, s_backBufferCount> m_frameFenceValues = {};
	std::array<uint64_t, s_backBufferCount> m_frameValues = {};

	// By default, enable V-Sync.
	// Can be toggled with the V key.
	bool m_isVSync = true;
	bool m_isTearingSupported = false;
	// By default, use windowed mode.
	// Can be toggled with the Alt+Enter or F11
	bool m_isFullscreen = false;
	bool m_isUsingWarp = false;

	FLOAT m_clientWidth;
	FLOAT m_clientHeight;

	HWND m_osWindowHandle;

	D3D12_VIEWPORT m_viewport = {};
	D3D12_RECT m_scissorRect = {};

	float m_foV = 45.f;

	matrix m_projectionMatrix;
	matrix m_orthogonalMatrix;
};

}
#endif
}
