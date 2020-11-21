// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "DX12CommandQueue.h"
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
	static constexpr FLOAT s_defaultClearColor[4] = { 0.f,0.f,0.f,1.f };

public:
	DX12Driver(HWND windowHandle, math::vec2<uint32_t> windowSz);
	~DX12Driver() = default;

private:
	void OnInitialize() override;
	void OnShutdown() override;
	void OnRender() override;

	void Flush();
	void ResizeDepthBuffer();

	static void UpdateBufferResource(
		Microsoft::WRL::ComPtr<ID3D12Device2> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource, 
		ID3D12Resource** pIntermediateResource,
		size_t numElements, 
		size_t elementSize, 
		const void* bufferData,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE
	);
	static void UpdateDepthBuffer(
		UINT clientW,
		UINT clientH,
		Microsoft::WRL::ComPtr<ID3D12Device2> device,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DSVHeap,
		Microsoft::WRL::ComPtr<ID3D12Resource>& outDepthBuffer
	);
	static void TransitionResource(
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		Microsoft::WRL::ComPtr<ID3D12Resource> resource,
		D3D12_RESOURCE_STATES beforeState,
		D3D12_RESOURCE_STATES afterState
	);
	static void ClearRTV(
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE rtv,
		const FLOAT* clearColor
	);
	static void ClearDSV(
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE dsv,
		FLOAT depth = 1.f
	);

	// DirectX 12 Objects
	Microsoft::WRL::ComPtr<ID3D12Device2> m_device;
	std::shared_ptr<DX12CommandQueue> m_directCommandQueue;
	std::shared_ptr<DX12CommandQueue> m_computeCommandQueue;
	std::shared_ptr<DX12CommandQueue> m_copyCommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, s_backBufferCount> m_backBuffers;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
	UINT m_RTVDescriptorSize = 0;
	UINT m_currentBackBufferIndex = 0;

	// Synchronization objects
	std::array<uint64_t, s_backBufferCount> m_frameFenceValues = {};

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
	FLOAT m_clientWidth;
	FLOAT m_clientHeight;

	HWND m_osWindowHandle;

	//////////////////////////////////////////
	// Vertex buffer for the cube.
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
	// Index buffer for the cube.
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

	// Depth buffer.
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthBuffer;
	// Descriptor heap for depth buffer.
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

	// Root signature
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

	// Pipeline state object.
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

	D3D12_VIEWPORT m_viewport = {};
	D3D12_RECT m_scissorRect = {};

	float m_foV = 45.f;

	DirectX::XMMATRIX m_modelMatrix;
	DirectX::XMMATRIX m_viewMatrix;
	DirectX::XMMATRIX m_projectionMatrix;
};

}
}
}