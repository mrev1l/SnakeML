// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "system/drivers/RenderDriver.h"

namespace snakeml
{
namespace system
{
namespace win
{

class DX12Driver
	: public IRenderDriver
{
	static constexpr std::chrono::milliseconds s_DX12FenceDefaultWait = std::chrono::milliseconds::max();
	static constexpr uint8_t s_backBufferCount = 2u;

public:
	DX12Driver(HWND windowHandle);
	~DX12Driver() = default;

private:
	void OnInitialize() override;
	void OnShutdown() override;
	void OnRender() override;

	// DirectX 12 Objects
	Microsoft::WRL::ComPtr<ID3D12Device2> m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, s_backBufferCount> m_backBuffers;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	std::array<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, s_backBufferCount> m_commandAllocators;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
	UINT m_RTVDescriptorSize = 0;
	UINT m_currentBackBufferIndex = 0;

	// Synchronization objects
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	uint64_t m_fenceValue = 0;
	std::array<uint64_t, s_backBufferCount> m_frameFenceValues = {};
	HANDLE m_fenceEvent = nullptr;

	// By default, enable V-Sync.
	// Can be toggled with the V key.
	bool m_isVSync = true;
	bool m_isTearingSupported = false;
	// By default, use windowed mode.
	// Can be toggled with the Alt+Enter or F11
	bool m_isFullscreen = false;
	bool m_isUsingWarp = false;
	bool m_isInitialized = false;

	RECT m_cachedWindowRect = RECT{};
	uint32_t m_clientWidth = 1280;
	uint32_t m_clientHeight = 720;

	HWND m_osWindowHandle;
};

}
}
}