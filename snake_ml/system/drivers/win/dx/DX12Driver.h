// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "DX12CommandQueue.h"
#include "DX12DescriptorAllocator.h"
#include "DX12RenderTarget.h"
#include "system/drivers/RenderDriver.h"
#include "system/ecs/Entity.h"

namespace snakeml
{
namespace system
{
namespace win
{

// An enum for root signature parameters.
// I'm not using scoped enums to avoid the explicit cast that would be required
// to use these as root indices in the root signature.
enum RootParameters
{
	MatricesCB,		 // ConstantBuffer<Mat> MatCB : register(b0);
	MaterialCB,		 // ConstantBuffer<Material> MaterialCB : register( b0, space1 );
	LightPropertiesCB,  // ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );
	PointLights,		// StructuredBuffer<PointLight> PointLights : register( t0 );
	SpotLights,		 // StructuredBuffer<SpotLight> SpotLights : register( t1 );
	Textures,		   // Texture2D DiffuseTexture : register( t2 );
	NumRootParameters
};

class DX12Driver
	: public IRenderDriver
{
	static constexpr std::chrono::milliseconds s_DX12FenceDefaultWait = std::chrono::milliseconds::max();
	static constexpr uint8_t s_backBufferCount = 2u;
	static constexpr FLOAT s_defaultClearColor[4] = { 0.f,0.f,0.f,1.f };

public:
	enum class CommandQueueType
	{
		Direct,
		Compute,
		Copy
	};

	DX12Driver(HWND windowHandle, math::vec2<uint32_t> windowSz);
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

	void GetMatrices(DirectX::XMMATRIX& outProjection, DirectX::XMMATRIX& outOrthogonal);

private:
	void OnInitialize() override;
	void OnShutdown() override;
	void OnRender() override;
	void OnRender_ClearRenderTargets(std::shared_ptr<DX12CommandList> commandList);
	void OnRender_Present(std::shared_ptr<DX12CommandList> commandList, std::shared_ptr<DX12CommandQueue> commandQueue);

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

	DirectX::XMMATRIX m_projectionMatrix;
	DirectX::XMMATRIX m_orthogonalMatrix;
};

}
}
}