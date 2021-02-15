// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "ECSManager.h"

#include "utils/snake_math.h"
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON // not sure i need this
#include <codecvt>
#include <stringapiset.h>

namespace snakeml
{
namespace system
{

class DX12MaterialComponent : public IComponent
{
public:
	struct VSParamLayout
	{
		UINT num32BitValues;
		UINT shaderRegister;
		UINT registerSpace = 0;
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;
	};

	void Serialize(std::unordered_map<std::string, EngineTypes>& blackboard) const override {}

	void Deserialize(const std::unordered_map<std::string, EngineTypes>& blackboard) override {}

	ComponentType GetComponentType() const override { return ComponentType::DX12MaterialComponent; }

	std::vector<std::pair<math::vec3<float>, math::vec3<float>>> m_vertices;
	std::vector<uint16_t> m_indices;
	std::wstring m_vs;
	std::wstring m_ps;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_vsInputLayout;
	VSParamLayout m_vsParamLayout;
};
REGISTER_TYPE(DX12MaterialComponent);

class DX12RenderComponent : public IComponent
{
public:
	void Serialize(std::unordered_map<std::string, EngineTypes>& blackboard) const override {}

	void Deserialize(const std::unordered_map<std::string, EngineTypes>& blackboard) override {}

	ComponentType GetComponentType() const override { return ComponentType::DX12RenderComponent; }

	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
};
REGISTER_TYPE(DX12RenderComponent);

class TransformComponent : public IComponent
{
public:
	void Serialize(std::unordered_map<std::string, EngineTypes>& blackboard) const override {}

	void Deserialize(const std::unordered_map<std::string, EngineTypes>& blackboard) override {}

	ComponentType GetComponentType() const override { return ComponentType::TransformComponent; }

	math::vec4<float> m_position;
	math::vec4<float> m_rotation;
	math::vec4<float> m_scale;
};
REGISTER_TYPE(TransformComponent);

class InitializeCubeMaterialSystem : public ISystem
{
public:
	void Execute() override
	{
		static std::vector<std::pair<math::vec3<float>, math::vec3<float>>> g_Vertices = {
		{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f} }, // 0
		{ {-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f} }, // 1
		{ {1.0f,  1.0f, -1.0f}, {1.0f, 1.0f, 0.0f} }, // 2
		{ {1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f} }, // 3
		{ {-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f} }, // 4
		{ {-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 1.0f} }, // 5
		{ {1.0f,  1.0f,  1.0f}, {1.0f, 1.0f, 1.0f} }, // 6
		{ {1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 1.0f} }  // 7
		};

		static std::vector<uint16_t> g_Indicies =
		{
			0, 1, 2, 0, 2, 3,
			4, 6, 5, 4, 7, 6,
			4, 5, 1, 4, 1, 0,
			3, 2, 6, 3, 6, 7,
			1, 5, 6, 1, 6, 2,
			4, 0, 3, 4, 3, 7
		};

		static std::vector<D3D12_INPUT_ELEMENT_DESC> g_vsInputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		static DX12MaterialComponent::VSParamLayout g_vsParamLayout =
		{
			sizeof(DirectX::XMMATRIX) / _countof(DirectX::XMMATRIX::r), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX
		};

		DX12MaterialComponentIterator* it = (DX12MaterialComponentIterator*)IComponent::CreateIterator(ComponentType::DX12MaterialComponent, 1);
		DX12MaterialComponent* material = (DX12MaterialComponent*)it->At(0);
		material->m_entityId = 0;
		material->m_vertices = g_Vertices;
		material->m_indices = g_Indicies;
		material->m_vs = L"VS_MvpColor.cso";
		material->m_ps = L"PS_Color.cso";
		material->m_vsInputLayout = g_vsInputLayout;
		material->m_vsParamLayout = g_vsParamLayout;

		ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::DX12MaterialComponent, it);
	}
};

class LoadMaterialsSystem : public ISystem
{
public:
	void Execute() override
	{
		constexpr uint32_t materialsNum = 1u;
		constexpr const char* jsonName = "D:\\Projects_Library\\snakeml_repo\\materialcomponentjson.txt"; // fix full path

		rapidjson::Document jsonDocument;
		std::string jsonString;
		WinUtils::LoadFileIntoBuffer(jsonName, jsonString);

		ParseJsonString(jsonString.c_str(), jsonDocument);

		DX12MaterialComponentIterator* it = (DX12MaterialComponentIterator*)IComponent::CreateIterator(ComponentType::DX12MaterialComponent, materialsNum);
		DX12MaterialComponent* material = (DX12MaterialComponent*)it->At(0);

		ParseEntityId(jsonDocument, material->m_entityId);
		ParseVerticesArray(jsonDocument, material->m_vertices);
		ParseIndicesArray(jsonDocument, material->m_indices);
		ParseVSName(jsonDocument, material->m_vs);
		ParsePSName(jsonDocument, material->m_ps);
		ParseVertexInputLayout(jsonDocument, material->m_vsInputLayout);
		ParseVertexInputParamLayout(jsonDocument, material->m_vsParamLayout);

		ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::DX12MaterialComponent, it);
	}

private:
	static void ParseJsonString(const char* jsonBuffer, rapidjson::Document& outJson)
	{
		/// jsonBuffer needs to have the same scope as outJson to utilize ParseInsitu
		ASSERT(!outJson.ParseInsitu(const_cast<char*>(jsonBuffer)).HasParseError(), "Failed to parse JSON"); // remove const cast => error C2664: 'void rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>::Free(void *)': cannot convert argument 1 from 'const char *' to 'void *'
		ASSERT(outJson.IsObject(), "Invalid JSON");
	}

	static void ParseEntityId(const rapidjson::Document& json, uint32_t& outId)
	{
		ASSERT(json.HasMember("entityId") && json["entityId"].IsUint(), "Invalid entityId json");
		outId = json["entityId"].GetUint();
	}

	static void ParseVerticesArray(const rapidjson::Document& json, std::vector<std::pair<math::vec3<float>, math::vec3<float>>>& outVertices)
	{
		ASSERT(json.HasMember("vertices") && json["vertices"].IsArray(), "Invalid vertices json");

		const rapidjson::GenericArray<true, rapidjson::Value>& verticesArray = json["vertices"].GetArray();
		rapidjson::Value::ConstValueIterator vertexIt = verticesArray.Begin();

		outVertices.resize(verticesArray.Size());
		for (auto& vertex : outVertices)
		{
			ASSERT(vertexIt->HasMember("pos") && (*vertexIt)["pos"].IsArray() && (*vertexIt)["pos"].Size() == 3u, "Invalid vertices json");
			ASSERT(vertexIt->HasMember("color") && (*vertexIt)["color"].IsArray() && (*vertexIt)["color"].Size() == 3u, "Invalid vertices json");

			const rapidjson::GenericArray<true, rapidjson::Value>& pos = (*vertexIt)["pos"].GetArray();
			const rapidjson::GenericArray<true, rapidjson::Value>& color = (*vertexIt)["color"].GetArray();

			vertex.first = { pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat() };
			vertex.second = { color[0].GetFloat(), color[1].GetFloat(), color[2].GetFloat() };

			++vertexIt;
		}
	}

	static void ParseIndicesArray(const rapidjson::Document& json, std::vector<uint16_t>& indicesArray)
	{
		ASSERT(json.HasMember("indices") && json["indices"].IsArray(), "Invalid indices json");
		const rapidjson::GenericArray<true, rapidjson::Value>& indicesJson = json["indices"].GetArray();

		indicesArray.resize(indicesJson.Size());
		rapidjson::Value::ConstValueIterator indicesIt = indicesJson.Begin();
		
		for (uint16_t& index : indicesArray)
		{
			index = (indicesIt++)->GetInt();
		}
	}

	static void ParseVSName(const rapidjson::Document& json, std::wstring& outVSName)
	{
		ASSERT(json.HasMember("vs") && json["vs"].IsString(), "Invalid vs json");

		std::string ps = json["vs"].GetString();
		wchar_t* wPSName = nullptr;
		snakeml::WinUtils::StringToWstring(ps.c_str(), wPSName);
		outVSName = std::wstring(wPSName); // TODO: we don't actually copy it in ctor do we?
		delete wPSName;
	}

	static void ParsePSName(const rapidjson::Document& json, std::wstring& outPSName)
	{
		ASSERT(json.HasMember("ps") && json["ps"].IsString(), "Invalid ps json");

		std::string ps = json["ps"].GetString();
		wchar_t* wPSName = nullptr;
		snakeml::WinUtils::StringToWstring(ps.c_str(), wPSName);
		outPSName = std::wstring(wPSName); // TODO: we don't actually copy it in ctor do we?
		delete wPSName;
	}

	static void ParseVertexInputLayout(const rapidjson::Document& json, std::vector<D3D12_INPUT_ELEMENT_DESC>& outLayout)
	{
		ASSERT(json.HasMember("vertex_input_layout") && json["vertex_input_layout"].IsArray(), "Invalid vertex_input_layout json");
		const rapidjson::GenericArray<true, rapidjson::Value>& vsInputJson = json["vertex_input_layout"].GetArray();

		outLayout.resize(vsInputJson.Size());
		rapidjson::Value::ConstValueIterator vsInputLayoutIt = vsInputJson.Begin();

		for (auto& inputElement : outLayout)
		{
			ASSERT(vsInputLayoutIt->HasMember("semanticName") && (*vsInputLayoutIt)["semanticName"].IsString(), "Invalid vertex_input_layout json");
			ASSERT(vsInputLayoutIt->HasMember("semanticIdx") && (*vsInputLayoutIt)["semanticIdx"].IsUint(), "Invalid vertex_input_layout json");
			ASSERT(vsInputLayoutIt->HasMember("format") && (*vsInputLayoutIt)["format"].IsInt(), "Invalid vertex_input_layout json");
			ASSERT(vsInputLayoutIt->HasMember("inputSlot") && (*vsInputLayoutIt)["inputSlot"].IsUint(), "Invalid vertex_input_layout json");
			ASSERT(vsInputLayoutIt->HasMember("alignedByteOffset") && (*vsInputLayoutIt)["alignedByteOffset"].IsUint(), "Invalid vertex_input_layout json");
			ASSERT(vsInputLayoutIt->HasMember("inputSlotClass") && (*vsInputLayoutIt)["inputSlotClass"].IsInt(), "Invalid vertex_input_layout json");
			ASSERT(vsInputLayoutIt->HasMember("instanceDataStepRate") && (*vsInputLayoutIt)["instanceDataStepRate"].IsUint(), "Invalid vertex_input_layout json");

			const std::string name = (*vsInputLayoutIt)["semanticName"].GetString();
			inputElement.SemanticName = new char[name.size() + 1]; // TODO: make sure to delete
			strcpy_s((char*)(inputElement.SemanticName), name.size() + 1, name.c_str());

			inputElement.SemanticIndex = (*vsInputLayoutIt)["semanticIdx"].GetUint();
			inputElement.Format = static_cast<DXGI_FORMAT>((*vsInputLayoutIt)["format"].GetInt());
			inputElement.InputSlot = (*vsInputLayoutIt)["inputSlot"].GetUint();
			inputElement.AlignedByteOffset = (*vsInputLayoutIt)["alignedByteOffset"].GetUint();
			inputElement.InputSlotClass = static_cast<D3D12_INPUT_CLASSIFICATION>((*vsInputLayoutIt)["inputSlotClass"].GetInt());
			inputElement.InstanceDataStepRate = (*vsInputLayoutIt)["instanceDataStepRate"].GetUint();

			++vsInputLayoutIt;
		}
	}

	static void ParseVertexInputParamLayout(const rapidjson::Document& json, DX12MaterialComponent::VSParamLayout& outLayout)
	{
		ASSERT(json.HasMember("vertex_input_param_layout") && json["vertex_input_param_layout"].IsObject(), "Invalid vertex_input_param_layout json");

		const rapidjson::Value& vsInputParamLayoutJson = json["vertex_input_param_layout"];
		ASSERT(vsInputParamLayoutJson.HasMember("num32BitValues") && vsInputParamLayoutJson["num32BitValues"].IsUint(), "Invalid vertex_input_param_layout json");
		ASSERT(vsInputParamLayoutJson.HasMember("shaderRegister") && vsInputParamLayoutJson["shaderRegister"].IsUint(), "Invalid vertex_input_param_layout json");
		ASSERT(vsInputParamLayoutJson.HasMember("registerSpace") && vsInputParamLayoutJson["registerSpace"].IsUint(), "Invalid vertex_input_param_layout json");
		ASSERT(vsInputParamLayoutJson.HasMember("visibility") && vsInputParamLayoutJson["visibility"].IsInt(), "Invalid vertex_input_param_layout json");

		outLayout.num32BitValues = vsInputParamLayoutJson["num32BitValues"].GetUint();
		outLayout.shaderRegister = vsInputParamLayoutJson["shaderRegister"].GetUint();
		outLayout.registerSpace = vsInputParamLayoutJson["registerSpace"].GetUint();
		outLayout.visibility = static_cast<D3D12_SHADER_VISIBILITY>(vsInputParamLayoutJson["visibility"].GetInt());
	}
};

class InitializeRenderComponentsSystem : public ISystem
{
public:
	void Execute() override
	{
		Iterator* materialComponents = ECSManager::GetInstance()->GetComponentsPool().GetComponents(ComponentType::DX12MaterialComponent);
		DX12MaterialComponent* materials = (DX12MaterialComponent*)materialComponents->GetData();

		DX12RenderComponentIterator* renderComponentsIt = (DX12RenderComponentIterator*)IComponent::CreateIterator(ComponentType::DX12RenderComponent, materialComponents->Num());
		ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::DX12RenderComponent, renderComponentsIt);
		DX12RenderComponent* renderComponents = (DX12RenderComponent*)renderComponentsIt->GetData();

		win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
		auto device = dx12Driver->GetD3D12Device();
		auto commandQueue = dx12Driver->GetDX12CommandQueue(win::DX12Driver::CommandQueueType::Copy);
		auto commandList = commandQueue->GetD3D12CommandList();

		for (int i = 0; i < materialComponents->Num(); ++i)
		{
			DX12RenderComponent& renderComponent = renderComponents[i];
			const DX12MaterialComponent& materialComponent = materials[i];

			renderComponent.m_entityId = materialComponent.m_entityId;
			// Upload vertex buffer data.
			Microsoft::WRL::ComPtr<ID3D12Resource> intermediateVertexBuffer;
			dxutils::UpdateBufferResource(device,
				commandList,
				&renderComponent.m_vertexBuffer, &intermediateVertexBuffer,
				materialComponent.m_vertices.size(), sizeof(materialComponent.m_vertices[0]), materialComponent.m_vertices.data());

			// Create the vertex buffer view.
			renderComponent.m_vertexBufferView.BufferLocation = renderComponent.m_vertexBuffer->GetGPUVirtualAddress();
			renderComponent.m_vertexBufferView.SizeInBytes = sizeof(materialComponent.m_vertices[0]) * materialComponent.m_vertices.size();
			renderComponent.m_vertexBufferView.StrideInBytes = sizeof(materialComponent.m_vertices[0]);

			// Upload index buffer data.
			Microsoft::WRL::ComPtr<ID3D12Resource> intermediateIndexBuffer;
			dxutils::UpdateBufferResource(device,
				commandList,
				&renderComponent.m_indexBuffer, &intermediateIndexBuffer,
				materialComponent.m_indices.size(), sizeof(materialComponent.m_indices[0]), materialComponent.m_indices.data());

			// Create index buffer view.
			renderComponent.m_indexBufferView.BufferLocation = renderComponent.m_indexBuffer->GetGPUVirtualAddress();
			renderComponent.m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
			renderComponent.m_indexBufferView.SizeInBytes = sizeof(materialComponent.m_indices[0]) * materialComponent.m_indices.size();

			// Load the vertex shader.
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
			dxutils::ThrowIfFailed(D3DReadFileToBlob(materialComponent.m_vs.data(), &vertexShaderBlob));
			// Load the pixel shader.
			Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
			dxutils::ThrowIfFailed(D3DReadFileToBlob(materialComponent.m_ps.data(), &pixelShaderBlob));

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
			rootParameters[0].InitAsConstants(
				materialComponent.m_vsParamLayout.num32BitValues,
				materialComponent.m_vsParamLayout.shaderRegister,
				materialComponent.m_vsParamLayout.registerSpace,
				materialComponent.m_vsParamLayout.visibility);

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
			rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);
			// Serialize the root signature.
			Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
			Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
			dxutils::ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
				featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

			// Create the root signature.
			dxutils::ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
				rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&renderComponent.m_rootSignature)));

			struct PipelineStateStream
			{
				CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
				CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
				CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
				CD3DX12_PIPELINE_STATE_STREAM_VS VS;
				CD3DX12_PIPELINE_STATE_STREAM_PS PS;
				CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
				CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
			} pipelineStateStream;

			D3D12_RT_FORMAT_ARRAY rtvFormats = {};
			rtvFormats.NumRenderTargets = 1;
			rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

			pipelineStateStream.pRootSignature = renderComponent.m_rootSignature.Get();
			pipelineStateStream.InputLayout = { materialComponent.m_vsInputLayout.data(), (UINT)materialComponent.m_vsInputLayout.size() };
			pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
			pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
			pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			pipelineStateStream.RTVFormats = rtvFormats;

			D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
			sizeof(PipelineStateStream), &pipelineStateStream
			};
			dxutils::ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&renderComponent.m_pipelineState)));

			auto fenceValue = commandQueue->ExecuteCommandList(commandList);
			commandQueue->WaitForFenceValue(fenceValue);
		}
	}
};

class InitializeTransformComponents : public ISystem
{
public:
	void Execute() override
	{
		TransformComponentIterator* it = (TransformComponentIterator*)IComponent::CreateIterator(ComponentType::TransformComponent, 1);
		TransformComponent* material = (TransformComponent*)it->At(0);
		material->m_entityId = 0;
		material->m_position = { 0.f, 0.f, 0.f, 1.f };
		material->m_rotation = { 0.f, 1.f, 1.f, 0.f };
		material->m_scale = { 1.f, 1.f, 1.f, 0.f };

		ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::TransformComponent, it);
	}
};

class InitializeEntitiesSystem : public ISystem
{
public:
	void Execute() override
	{
		const ComponentsPool& componentsPool = ECSManager::GetInstance()->GetComponentsPool();
		std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();

		for (size_t i = static_cast<size_t>(ComponentType::DX12MaterialComponent);
			i < static_cast<size_t>(ComponentType::Size);
			++i)
		{
			Iterator* it = componentsPool.GetComponents(static_cast<ComponentType>(i));
			for (size_t componentIdx = 0; componentIdx < it->Num(); ++componentIdx)
			{
				IComponent* component = it->At(componentIdx);

				std::vector<Entity>::iterator entityWithThisId = std::find_if(entities.begin(), entities.end(),
					[component](const Entity& a) { return a.m_entityId == component->m_entityId; });
				if (entityWithThisId == entities.end())
				{
					Entity newEntity;
					newEntity.m_entityId = component->m_entityId;
					newEntity.m_components.insert({ component->GetComponentType(), component });
					entities.push_back(newEntity);
				}
				else
				{
					entityWithThisId->m_components.insert({ component->GetComponentType(), component });
				}
			}
		}
	}
};

class RenderSystem : public ISystem
{
public:
	void Execute() override
	{
		win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
		const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
		for (const Entity& entity : entities)
		{
			const DX12RenderComponent& renderable = *(DX12RenderComponent*)entity.m_components.at(ComponentType::DX12RenderComponent);
			const TransformComponent& transform = *(TransformComponent*)entity.m_components.at(ComponentType::TransformComponent);

			auto commandQueue = dx12Driver->GetDX12CommandQueue(win::DX12Driver::CommandQueueType::Direct);
			auto commandList = commandQueue->GetD3D12CommandList();

			auto backBuffer = dx12Driver->m_backBuffers[dx12Driver->m_currentBackBufferIndex];
			auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(dx12Driver->m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), dx12Driver->m_currentBackBufferIndex, dx12Driver->m_RTVDescriptorSize);
			auto dsv = dx12Driver->m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

			// Clear the render targets.
			{
				dxutils::TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

				float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
				dxutils::ClearRTV(commandList, rtv, color);
				dxutils::ClearDSV(commandList, dsv);
			}

			commandList->SetPipelineState(renderable.m_pipelineState.Get());
			commandList->SetGraphicsRootSignature(renderable.m_rootSignature.Get());

			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->IASetVertexBuffers(0, 1, &renderable.m_vertexBufferView);
			commandList->IASetIndexBuffer(&renderable.m_indexBufferView);

			commandList->RSSetViewports(1, &dx12Driver->m_viewport);
			commandList->RSSetScissorRects(1, &dx12Driver->m_scissorRect);

			commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

			// Update the MVP matrix
			static std::chrono::high_resolution_clock clock;
			static auto t0 = clock.now();

			auto t1 = clock.now(); //-V656
			auto deltaTime = t1 - t0;

			float angle = static_cast<float>(deltaTime.count() * 1e-9 * 90.0);

			const DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(transform.m_rotation.m_x, transform.m_rotation.m_y, transform.m_rotation.m_z, transform.m_rotation.m_w);
			const DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));
			DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixMultiply(modelMatrix, dx12Driver->m_viewMatrix);
			mvpMatrix = DirectX::XMMatrixMultiply(mvpMatrix, dx12Driver->m_projectionMatrix);
			commandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / _countof(DirectX::XMMATRIX::r), &mvpMatrix, 0);

			commandList->DrawIndexedInstanced(renderable.m_indexBufferView.SizeInBytes / sizeof(uint16_t), 1, 0, 0, 0);

			// Present
			{
				dxutils::TransitionResource(commandList, backBuffer,
					D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

				dx12Driver->m_frameFenceValues[dx12Driver->m_currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);

				{
					UINT syncInterval = dx12Driver->m_isVSync ? 1 : 0;
					UINT presentFlags = dx12Driver->m_isTearingSupported && !dx12Driver->m_isVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
					dxutils::ThrowIfFailed(dx12Driver->m_swapChain->Present(syncInterval, presentFlags));
					dx12Driver->m_currentBackBufferIndex = dx12Driver->m_swapChain->GetCurrentBackBufferIndex();
				}

				commandQueue->WaitForFenceValue(dx12Driver->m_frameFenceValues[dx12Driver->m_currentBackBufferIndex]);
			}
		}
	}
};

class UpdateCubeSystem : public ISystem
{
public:
	void Execute() override
	{
		static std::chrono::high_resolution_clock clock;
		static auto t0 = clock.now();

		auto t1 = clock.now(); //-V656
		auto deltaTime = t1 - t0;

		const float angle = static_cast<float>(deltaTime.count() * 1e-9 * 90.0);

		const uint32_t entityIdToUpdate = 0;

		const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
		const std::vector<Entity>::const_iterator entityIt = std::find_if(entities.begin(), entities.end(),
			[entityIdToUpdate](const Entity& a) {return a.m_entityId == entityIdToUpdate; });
		if (entityIt != entities.end())
		{
			TransformComponent& transform = *(TransformComponent*)(entityIt->m_components.at(ComponentType::TransformComponent));
			auto scale = DirectX::XMMatrixScalingFromVector({ transform.m_scale.m_x, transform.m_scale.m_y, transform.m_scale.m_z, transform.m_scale.m_w });
			auto rotation = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(transform.m_rotation.m_x), DirectX::XMConvertToRadians(transform.m_rotation.m_y), DirectX::XMConvertToRadians(transform.m_rotation.m_z));
			auto translation = DirectX::XMMatrixTranslationFromVector({ transform.m_position.m_x, transform.m_position.m_y, transform.m_position.m_z, transform.m_position.m_w });
			auto transformMatrix = scale * rotation * translation;

			auto updatedRotation = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(0.f), DirectX::XMConvertToRadians(0.f), DirectX::XMConvertToRadians(angle));
			transformMatrix = transformMatrix * updatedRotation;

			DirectX::XMVECTOR newScale, newRotation, newTranslation;
			DirectX::XMMatrixDecompose(&newScale, &newRotation, &newTranslation, transformMatrix);

			DirectX::XMVECTOR pitchAxis = {1.0f, 0.f, 0.f, 0.f};
			float pitchAngle = 0.f;
			DirectX::XMVECTOR yawAxis = {0.0f, 1.f, 0.f, 0.f};
			float yawAngle = 0.f;
			DirectX::XMVECTOR rollAxis = {0.0f, 0.f, 1.f, 0.f};
			float rollAngle = 0.f;
			DirectX::XMQuaternionToAxisAngle(&yawAxis, &pitchAngle, newRotation);
			DirectX::XMQuaternionToAxisAngle(&yawAxis, &yawAngle, newRotation);
			DirectX::XMQuaternionToAxisAngle(&yawAxis, &rollAngle, newRotation);

			float newPitchAngle = DirectX::XMConvertToDegrees(pitchAngle);
			float newYawAngle = DirectX::XMConvertToDegrees(yawAngle);
			float newRollAngle = DirectX::XMConvertToDegrees(rollAngle);

			int stop = 23;
		}
	}
};

}
}