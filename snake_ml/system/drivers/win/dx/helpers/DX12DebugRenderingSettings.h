#pragma once

#include "system/drivers/win/dx/pipeline/DX12RootSignature.h"
#include "system/drivers/win/dx/resource/DX12VertexBuffer.h"

namespace snakeml
{

#ifdef _WINDOWS
namespace win
{
class DX12CommandList;

class DX12DebugRenderingSettings
{
public:
	DX12DebugRenderingSettings() = default;
	void Init();

	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState() const { return m_debugAABBPipelineState; }
	const DX12VertexBuffer& GetVertexBuffer() const { return m_debugAABBVertexBuffer; }
	const DX12RootSignature& GetRootSignature() const { return m_debugAABBRootSignature; }

private:
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER Rasterizer;
	};

	void Init_AABBSettings();
	void Init_AABBVertexBuffer(std::shared_ptr<DX12CommandList> commandList);
	void Init_AABBRootSignature(Microsoft::WRL::ComPtr<ID3D12Device2> device);
	void Init_PipelineState(Microsoft::WRL::ComPtr<ID3D12Device2> device, DXGI_SAMPLE_DESC sampleDesc);

	static std::vector<CD3DX12_ROOT_PARAMETER1> CreateAABBRootParameters();
	static CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT CreateAABBInputLayout();

	static D3D12_FEATURE_DATA_ROOT_SIGNATURE CreateFeatureData(Microsoft::WRL::ComPtr<ID3D12Device2> device);

	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_debugAABBPipelineState;
	DX12VertexBuffer m_debugAABBVertexBuffer;
	DX12RootSignature m_debugAABBRootSignature;

	static std::vector<std::pair<float3, float3>> s_debugAABBVertices;
	static D3D12_INPUT_ELEMENT_DESC s_aabbInputLayoutDefinition[2];
	static constexpr const wchar_t* s_debugVSPath = L"data/shaders/VS_MvpColor.cso";
	static constexpr const wchar_t* s_debugPSPath = L"data/shaders/PS_Color.cso";
	static constexpr UINT s_modelMatrixNumValues = 16u;
};

}
#endif

}