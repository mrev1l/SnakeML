// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12Driver.h"

#include "system/drivers/win/os/WinDriver.h"
#include "utils/win_utils.h"

namespace snakeml
{
namespace system
{
namespace win
{
	// Vertex data for a colored cube.
	struct VertexPosColor
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Color;
	};

	static VertexPosColor g_Vertices[8] = {
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
		{ DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
		{ DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
		{ DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
		{ DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
		{ DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
		{ DirectX::XMFLOAT3(1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
		{ DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
	};

	static WORD g_Indicies[36] =
	{
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7
	};

DX12Driver::DX12Driver(HWND windowHandle, math::vec2<uint32_t> windowSz)
	: IRenderDriver()
	, m_osWindowHandle(windowHandle)
	, m_clientWidth(static_cast<FLOAT>(windowSz.m_x))
	, m_clientHeight(static_cast<FLOAT>(windowSz.m_y))
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
	// Check for DirectX Math library support.
	if (!DirectX::XMVerifyCPUSupport())
	{
		MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter4 = dxutils::GetAdapter(m_isUsingWarp);
	m_device = dxutils::CreateDevice(dxgiAdapter4);
	m_directCommandQueue = std::make_shared<DX12CommandQueue>(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_computeCommandQueue = std::make_shared<DX12CommandQueue>(m_device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	m_copyCommandQueue = std::make_shared<DX12CommandQueue>(m_device, D3D12_COMMAND_LIST_TYPE_COPY);
	m_swapChain = dxutils::CreateSwapChain(m_osWindowHandle, m_directCommandQueue->GetD3D12CommandQueue(), 
		static_cast<uint32_t>(m_clientWidth), static_cast<uint32_t>(m_clientHeight), s_backBufferCount);
	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	m_RTVDescriptorHeap = dxutils::CreateDescriptorHeap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, s_backBufferCount);
	m_RTVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	dxutils::UpdateRenderTargetViews(m_device, m_swapChain, m_RTVDescriptorHeap, s_backBufferCount, m_backBuffers.data());

	// Create the descriptor heap for the depth-stencil view.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dxutils::ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, m_clientWidth, m_clientHeight);
	m_scissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);

	m_orthogonalMatrix = DirectX::XMMatrixOrthographicLH(m_clientWidth, m_clientHeight, 0.1f, 100.f);
	
	// Resize/Create the depth buffer.
	ResizeDepthBuffer();

	///////////////////////////////////////
	// LOAD CONTENT
	///////////////////////////////////////
	{
		auto device = m_device;
		auto commandQueue = m_copyCommandQueue;
		auto commandList = commandQueue->GetD3D12CommandList();

		// Upload vertex buffer data.
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateVertexBuffer;
		UpdateBufferResource(device,
			commandList,
			&m_vertexBuffer, &intermediateVertexBuffer,
			_countof(g_Vertices), sizeof(VertexPosColor), g_Vertices);

		// Create the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = sizeof(g_Vertices);
		m_vertexBufferView.StrideInBytes = sizeof(VertexPosColor);

		// Upload index buffer data.
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateIndexBuffer;
		UpdateBufferResource(device,
			commandList,
			&m_indexBuffer, &intermediateIndexBuffer,
			_countof(g_Indicies), sizeof(WORD), g_Indicies);

		// Create index buffer view.
		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		m_indexBufferView.SizeInBytes = sizeof(g_Indicies);


		// Load the vertex shader.
		Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
		dxutils::ThrowIfFailed(D3DReadFileToBlob(L"VS_MvpColor.cso", &vertexShaderBlob));
		// Load the pixel shader.
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
		dxutils::ThrowIfFailed(D3DReadFileToBlob(L"PS_Color.cso", &pixelShaderBlob));

		// Create the vertex input layout
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// Create a root signature.
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// Allow input layout and deny unnecessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		// A single 32-bit constant root parameter that is used by the vertex shader.
		CD3DX12_ROOT_PARAMETER1 rootParameters[1] = { };
		rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / _countof(DirectX::XMMATRIX::r), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		// Serialize the root signature.
		Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		dxutils::ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
			featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

		// Create the root signature.
		dxutils::ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

		struct PipelineStateStream
		{
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
			CD3DX12_PIPELINE_STATE_STREAM_VS VS;
			CD3DX12_PIPELINE_STATE_STREAM_PS PS;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DS;
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		} pipelineStateStream;

		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		rtvFormats.NumRenderTargets = 1;
		rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc;
		depthStencilDesc.DepthEnable = false;// true;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		depthStencilDesc.StencilEnable = true;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;
		depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		pipelineStateStream.pRootSignature = m_rootSignature.Get();
		pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
		pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
		pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
		pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		pipelineStateStream.DS = depthStencilDesc;
		pipelineStateStream.RTVFormats = rtvFormats;

		D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
		};
		dxutils::ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_pipelineState)));

		auto fenceValue = commandQueue->ExecuteCommandList(commandList);
		commandQueue->WaitForFenceValue(fenceValue);
	}
}

void DX12Driver::OnShutdown()
{
	Flush();
}

void DX12Driver::OnRender()
{
	// Temp Update
	{
		static std::chrono::high_resolution_clock clock;
		static auto t0 = clock.now();

		auto t1 = clock.now(); //-V656
		auto deltaTime = t1 - t0;
		
		float angle = static_cast<float>(deltaTime.count() * 1e-9 * 90.0);
		const DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);
		m_modelMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));

		// Update the view matrix.
		const DirectX::XMVECTOR eyePosition = DirectX::XMVectorSet(0, 0, -10, 1);
		const DirectX::XMVECTOR focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
		const DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
		m_viewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

		// Update the projection matrix.
		float aspectRatio = static_cast<float>(m_clientWidth) / static_cast<float>(m_clientHeight);
		m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_foV), aspectRatio, 0.1f, 100.0f);
	}
	return;
	// Render
	{
		auto commandQueue = m_directCommandQueue;
		auto commandList = commandQueue->GetD3D12CommandList();

		auto backBuffer = m_backBuffers[m_currentBackBufferIndex];
		auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_currentBackBufferIndex, m_RTVDescriptorSize);
		auto dsv = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

		// Clear the render targets.
		{
			TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

			ClearRTV(commandList, rtv, s_defaultClearColor);
			ClearDSV(commandList, dsv);
		}

		commandList->SetPipelineState(m_pipelineState.Get());
		commandList->SetGraphicsRootSignature(m_rootSignature.Get());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		commandList->IASetIndexBuffer(&m_indexBufferView);

		commandList->RSSetViewports(1, &m_viewport);
		commandList->RSSetScissorRects(1, &m_scissorRect);

		commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

		// Update the MVP matrix
		DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixMultiply(m_modelMatrix, m_viewMatrix);
		mvpMatrix = DirectX::XMMatrixMultiply(mvpMatrix, m_projectionMatrix);
		commandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / _countof(DirectX::XMMATRIX::r), &mvpMatrix, 0);

		commandList->DrawIndexedInstanced(_countof(g_Indicies), 1, 0, 0, 0);

		// Present
		{
			TransitionResource(commandList, backBuffer,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			m_frameFenceValues[m_currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);

			{
				UINT syncInterval = m_isVSync ? 1 : 0;
				UINT presentFlags = m_isTearingSupported && !m_isVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
				dxutils::ThrowIfFailed(m_swapChain->Present(syncInterval, presentFlags));
				m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
			}

			commandQueue->WaitForFenceValue(m_frameFenceValues[m_currentBackBufferIndex]);
		}
	}
}

void DX12Driver::Flush()
{
	m_directCommandQueue->Flush();
	m_computeCommandQueue->Flush();
	m_copyCommandQueue->Flush();
}

void DX12Driver::ResizeDepthBuffer()
{
	// Flush any GPU commands that might be referencing the depth buffer.
	Flush();

	UpdateDepthBuffer(static_cast<UINT>(m_clientWidth), static_cast<UINT>(m_clientHeight), m_device, m_DSVHeap, m_depthBuffer);
}

void DX12Driver::UpdateBufferResource(
	Microsoft::WRL::ComPtr<ID3D12Device2> device, 
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, 
	ID3D12Resource** pDestinationResource, 
	ID3D12Resource** pIntermediateResource, 
	size_t numElements, 
	size_t elementSize, 
	const void* bufferData, 
	D3D12_RESOURCE_FLAGS flags)
{
	const size_t bufferSize = numElements * elementSize;

	// Create a committed resource for the GPU resource in a default heap.
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT); // !!!!????
	CD3DX12_RESOURCE_DESC resourceDesc(CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags)); // !!!!????
	dxutils::ThrowIfFailed(device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(pDestinationResource)));

	// Create an committed resource for the upload.
	if (bufferData)
	{
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD); // !!!!????
		CD3DX12_RESOURCE_DESC resourceDesc(CD3DX12_RESOURCE_DESC::Buffer(bufferSize)); // !!!!????
		dxutils::ThrowIfFailed(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(pIntermediateResource)));

		D3D12_SUBRESOURCE_DATA subresourceData = { bufferData, static_cast<LONG_PTR>(bufferSize), subresourceData.RowPitch };

		UpdateSubresources(commandList.Get(), *pDestinationResource, *pIntermediateResource, 0, 0, 1, &subresourceData);
	}
}

void DX12Driver::UpdateDepthBuffer(
	UINT clientW,
	UINT clientH,
	Microsoft::WRL::ComPtr<ID3D12Device2> device, 
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DSVHeap, 
	Microsoft::WRL::ComPtr<ID3D12Resource>& outDepthBuffer)
{
	// Resize screen dependent resources.
	// Create a depth buffer.
	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	optimizedClearValue.DepthStencil = { 1.0f, 0 };

	CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC tex2D(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, clientW, clientH, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));
	dxutils::ThrowIfFailed(device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&tex2D,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optimizedClearValue,
		IID_PPV_ARGS(&outDepthBuffer)
	));

	// Update the depth-stencil view.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsv = { DXGI_FORMAT_D32_FLOAT, D3D12_DSV_DIMENSION_TEXTURE2D, D3D12_DSV_FLAG_NONE };
	dsv.Texture2D.MipSlice = 0;	
	device->CreateDepthStencilView(outDepthBuffer.Get(), &dsv, DSVHeap->GetCPUDescriptorHandleForHeapStart());
}

void DX12Driver::TransitionResource(
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, 
	Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
	D3D12_RESOURCE_STATES beforeState, 
	D3D12_RESOURCE_STATES afterState)
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), beforeState, afterState);
	commandList->ResourceBarrier(1, &barrier);
}

void DX12Driver::ClearRTV(
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, 
	D3D12_CPU_DESCRIPTOR_HANDLE rtv, 
	const FLOAT* clearColor)
{
	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void DX12Driver::ClearDSV(
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, 
	D3D12_CPU_DESCRIPTOR_HANDLE dsv, 
	FLOAT depth)
{
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

}
}
}
