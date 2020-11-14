// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12Driver.h"

#include "system/drivers/win/WinDriver.h"
#include "utils/win_utils.h"

namespace snakeml
{
namespace system
{
namespace win
{

DX12Driver::DX12Driver(HWND windowHandle)
	: IRenderDriver()
	, m_osWindowHandle(windowHandle)
{
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

	Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter4 = dxutils::GetAdapter(m_isUsingWarp);
	m_device = dxutils::CreateDevice(dxgiAdapter4);
	m_commandQueue = dxutils::CreateCommandQueue(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_swapChain = dxutils::CreateSwapChain(m_osWindowHandle, m_commandQueue, m_clientWidth, m_clientHeight, s_backBufferCount);
	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	m_RTVDescriptorHeap = dxutils::CreateDescriptorHeap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, s_backBufferCount);
	m_RTVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	dxutils::UpdateRenderTargetViews(m_device, m_swapChain, m_RTVDescriptorHeap, s_backBufferCount, m_backBuffers.data());

	for (int i = 0; i < s_backBufferCount; ++i)
	{
		m_commandAllocators[i] = dxutils::CreateCommandAllocator(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	}

	m_commandList = dxutils::CreateCommandList(m_device,
		m_commandAllocators[m_currentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

	m_fence = dxutils::CreateFence(m_device);
	m_fenceEvent = WinUtils::CreateEventHandle();
}

void DX12Driver::OnShutdown()
{
	// Make sure the command queue has finished all commands before closing.
	dxutils::Flush(m_commandQueue, m_fence, m_fenceValue, m_fenceEvent, s_DX12FenceDefaultWait);

	::CloseHandle(m_fenceEvent);
}

void DX12Driver::OnRender()
{
	auto commandAllocator = m_commandAllocators[m_currentBackBufferIndex];
	auto backBuffer = m_backBuffers[m_currentBackBufferIndex];

	commandAllocator->Reset();
	m_commandList->Reset(commandAllocator.Get(), nullptr);
	// Clear the render target.
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		m_commandList->ResourceBarrier(1, &barrier);

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			static_cast<INT>(m_currentBackBufferIndex), m_RTVDescriptorSize);

		m_commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	}

	// Present
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_commandList->ResourceBarrier(1, &barrier);
		dxutils::ThrowIfFailed(m_commandList->Close());

		ID3D12CommandList* const commandLists[] = {
			m_commandList.Get()
		};
		m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		UINT syncInterval = m_isVSync ? 1u : 0u;
		UINT presentFlags = m_isTearingSupported && !m_isVSync ? DXGI_PRESENT_ALLOW_TEARING : 0u;
		dxutils::ThrowIfFailed(m_swapChain->Present(syncInterval, presentFlags));

		m_frameFenceValues[m_currentBackBufferIndex] = dxutils::Signal(m_commandQueue, m_fence, m_fenceValue);
		m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

		dxutils::WaitForFenceValue(m_fence, m_frameFenceValues[m_currentBackBufferIndex], m_fenceEvent, s_DX12FenceDefaultWait);
	}
}

}
}
}
