// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "utils/patterns/singleton.h"

#include <memory>
#include <unordered_map>
#include <variant>
#include <string_view>

#include "lib_includes/directX_includes.h"
#include "system/drivers/win/dx/DX12Driver.h"

namespace snakeml
{
namespace system
{

using EngineTypes = std::variant<int32_t, uint16_t, float, std::string_view>;

enum class ComponentType : uint32_t
{
	DX12MaterialComponent = 0,
	DX12RenderComponent,
	TransformComponent,
	Size
};

#define REGISTER_TYPE(ObjectType) \
	class ObjectType##Iterator : public Iterator { \
	public: \
		ObjectType##Iterator(IComponent* data, size_t num) : Iterator(data, num) {} \
		\
	protected: \
		IComponent* GetElement(size_t idx) override { \
			ObjectType* concreteArray = (ObjectType*)m_data; \
			return &concreteArray[idx]; \
		} \
	}; \
	\
    class ObjectType##Factory : public Factory { \
    public: \
        ObjectType##Factory() \
        { \
            IComponent::RegisterFactory(ComponentType::##ObjectType, this); \
        } \
        virtual IComponent* Create() { \
            return new ObjectType(); \
        } \
        virtual IComponent* Create(size_t num) { \
            return new ObjectType[num]; \
        } \
		virtual Iterator* CreateIterator(size_t num) { \
			return new ObjectType##Iterator(new ObjectType[num](), num); \
		} \
}; \
static ObjectType##Factory global_##ObjectType##Factory; \

class IComponent;

class Iterator
{
public:
	Iterator(IComponent* data, size_t num) : m_data(data), m_count(num) {};
	IComponent* At(size_t idx)
	{
		return GetElement(idx);
	}

	IComponent* GetData() { return m_data; }
	size_t Num() { return m_count; }

protected:
	virtual IComponent* GetElement(size_t idx) = 0;

	IComponent* m_data;
	size_t m_count;
};

class Factory
{
public:
	virtual IComponent* Create() = 0;
	virtual IComponent* Create(size_t num) = 0;
	virtual Iterator* CreateIterator(size_t num) = 0;
};

class IComponent
{
public:
	virtual void Serialize(std::unordered_map<std::string, EngineTypes>&) const = 0;
	virtual void Deserialize(const std::unordered_map<std::string, EngineTypes>&) = 0;
	virtual ComponentType GetComponentType() const = 0;

	static void RegisterFactory(ComponentType objType, Factory* objFactory)
	{
		factories.insert({ objType, objFactory });
	}

	static IComponent* Create(ComponentType objType)
	{
		return factories.at(objType)->Create();
	}

	static IComponent* Create(ComponentType objType, size_t num)
	{
		return factories.at(objType)->Create(num);
	}

	static Iterator* CreateIterator(ComponentType objType, size_t num)
	{
		return factories.at(objType)->CreateIterator(num);
	}

	uint32_t m_entityId;

private:
	inline static std::unordered_map<ComponentType, Factory*> factories;
};
//std::unordered_map<ComponentType, Factory*> IComponent::factories;

//class DX12MaterialComponent : public IComponent
//{
//public:
//	struct VSParamLayout
//	{
//		UINT num32BitValues;
//		UINT shaderRegister;
//		UINT registerSpace = 0;
//		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;
//	};
//
//	void Serialize(std::unordered_map<std::string, EngineTypes>& blackboard) const override {}
//
//	void Deserialize(const std::unordered_map<std::string, EngineTypes>& blackboard) override {}
//
//	ComponentType GetComponentType() const override { return ComponentType::DX12MaterialComponent; }
//
//	std::vector<std::pair<math::vec3<float>, math::vec3<float>>> m_vertices;
//	std::vector<uint16_t> m_indices;
//	std::wstring m_vs;
//	std::wstring m_ps;
//	std::vector<D3D12_INPUT_ELEMENT_DESC> m_vsInputLayout;
//	VSParamLayout m_vsParamLayout;
//};
//REGISTER_TYPE(DX12MaterialComponent);
//
//class DX12RenderComponent : public IComponent
//{
//public:
//	void Serialize(std::unordered_map<std::string, EngineTypes>& blackboard) const override {}
//
//	void Deserialize(const std::unordered_map<std::string, EngineTypes>& blackboard) override {}
//
//	ComponentType GetComponentType() const override { return ComponentType::DX12RenderComponent; }
//
//	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
//	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
//	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
//	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
//	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
//	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
//};
//REGISTER_TYPE(DX12RenderComponent);
//
//class TransformComponent : public IComponent
//{
//public:
//	void Serialize(std::unordered_map<std::string, EngineTypes>& blackboard) const override {}
//
//	void Deserialize(const std::unordered_map<std::string, EngineTypes>& blackboard) override {}
//
//	ComponentType GetComponentType() const override { return ComponentType::TransformComponent; }
//
//	math::vec4<float> m_position;
//	math::vec4<float> m_rotation;
//	math::vec4<float> m_scale;
//};
//REGISTER_TYPE(TransformComponent);

class ComponentsPool
{
public:
	ComponentsPool() = default;

	void InsertComponents(ComponentType type, Iterator* it)
	{
		ASSERT(!m_componentsPool.contains(type), "Inserting into an existing component pool");
		m_componentsPool.insert({type, it});
	}
	Iterator* GetComponents(ComponentType type) const
	{
		return m_componentsPool.at(type);
	}

private:
	std::unordered_map<ComponentType, Iterator*> m_componentsPool;
};

class ISystem
{
public:
	virtual void Execute() = 0;
};

class Entity
{
public:
	uint32_t m_entityId;
	std::unordered_map<ComponentType, IComponent*> m_components;
};

class ECSManager : public patterns::Singleton<ECSManager>
{
public:
	ECSManager();

	ComponentsPool& GetComponentsPool()
	{
		return m_components;
	}

	std::vector<Entity>& GetEntities()
	{
		return m_entities;
	}

	void ExecuteSystem(ISystem* system)
	{
		system->Execute();
	}

	void ScheduleSystem(ISystem* system)
	{
		m_systems.push_back(system);
	}

	void Update()
	{
		for (ISystem* system : m_systems)
		{
			system->Execute();
		}
	}

private:
	ComponentsPool m_components;
	std::vector<Entity> m_entities;
	std::vector<ISystem*> m_systems;
};

//class InitializeCubeMaterialSystem : public ISystem
//{
//public:
//	void Execute() override
//	{
//		static std::vector<std::pair<math::vec3<float>, math::vec3<float>>> g_Vertices = {
//		{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f} }, // 0
//		{ {-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f} }, // 1
//		{ {1.0f,  1.0f, -1.0f}, {1.0f, 1.0f, 0.0f} }, // 2
//		{ {1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f} }, // 3
//		{ {-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f} }, // 4
//		{ {-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 1.0f} }, // 5
//		{ {1.0f,  1.0f,  1.0f}, {1.0f, 1.0f, 1.0f} }, // 6
//		{ {1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 1.0f} }  // 7
//		};
//
//		static std::vector<uint16_t> g_Indicies =
//		{
//			0, 1, 2, 0, 2, 3,
//			4, 6, 5, 4, 7, 6,
//			4, 5, 1, 4, 1, 0,
//			3, 2, 6, 3, 6, 7,
//			1, 5, 6, 1, 6, 2,
//			4, 0, 3, 4, 3, 7
//		};
//
//		static std::vector<D3D12_INPUT_ELEMENT_DESC> g_vsInputLayout =
//		{
//			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
//			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
//		};
//
//		static DX12MaterialComponent::VSParamLayout g_vsParamLayout =
//		{
//			sizeof(DirectX::XMMATRIX) / _countof(DirectX::XMMATRIX::r), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX
//		};
//
//		DX12MaterialComponentIterator* it = (DX12MaterialComponentIterator*)IComponent::CreateIterator(ComponentType::DX12MaterialComponent, 1);
//		DX12MaterialComponent* material = (DX12MaterialComponent*)it->At(0);
//		material->m_entityId = 0;
//		material->m_vertices = g_Vertices;
//		material->m_indices = g_Indicies;
//		material->m_vs = L"VS_MvpColor.cso";
//		material->m_ps = L"PS_Color.cso";
//		material->m_vsInputLayout = g_vsInputLayout;
//		material->m_vsParamLayout = g_vsParamLayout;
//
//		ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::DX12MaterialComponent, it);
//	}
//};
//
//class InitializeRenderComponentsSystem : public ISystem
//{
//public:
//	void Execute() override
//	{
//		Iterator* materialComponents = ECSManager::GetInstance()->GetComponentsPool().GetComponents(ComponentType::DX12MaterialComponent);
//		DX12MaterialComponent* materials = (DX12MaterialComponent*)materialComponents->GetData();
//
//		DX12RenderComponentIterator* renderComponentsIt = (DX12RenderComponentIterator*)IComponent::CreateIterator(ComponentType::DX12RenderComponent, materialComponents->Num());
//		ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::DX12RenderComponent, renderComponentsIt);
//		DX12RenderComponent* renderComponents = (DX12RenderComponent*)renderComponentsIt->GetData();
//
//		win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
//		auto device = dx12Driver->GetD3D12Device();
//		auto commandQueue = dx12Driver->GetDX12CommandQueue(win::DX12Driver::CommandQueueType::Copy);
//		auto commandList = commandQueue->GetD3D12CommandList();
//
//		for (int i = 0; i < materialComponents->Num(); ++i)
//		{
//			DX12RenderComponent& renderComponent = renderComponents[i];
//			const DX12MaterialComponent& materialComponent = materials[i];
//
//			renderComponent.m_entityId = materialComponent.m_entityId;
//			// Upload vertex buffer data.
//			Microsoft::WRL::ComPtr<ID3D12Resource> intermediateVertexBuffer;
//			dxutils::UpdateBufferResource(device,
//				commandList,
//				&renderComponent.m_vertexBuffer, &intermediateVertexBuffer,
//				materialComponent.m_vertices.size(), sizeof(materialComponent.m_vertices[0]), materialComponent.m_vertices.data());
//
//			// Create the vertex buffer view.
//			renderComponent.m_vertexBufferView.BufferLocation = renderComponent.m_vertexBuffer->GetGPUVirtualAddress();
//			renderComponent.m_vertexBufferView.SizeInBytes = sizeof(materialComponent.m_vertices[0]) * materialComponent.m_vertices.size();
//			renderComponent.m_vertexBufferView.StrideInBytes = sizeof(materialComponent.m_vertices[0]);
//
//			// Upload index buffer data.
//			Microsoft::WRL::ComPtr<ID3D12Resource> intermediateIndexBuffer;
//			dxutils::UpdateBufferResource(device,
//				commandList,
//				&renderComponent.m_indexBuffer, &intermediateIndexBuffer,
//				materialComponent.m_indices.size(), sizeof(materialComponent.m_indices[0]), materialComponent.m_indices.data());
//
//			// Create index buffer view.
//			renderComponent.m_indexBufferView.BufferLocation = renderComponent.m_indexBuffer->GetGPUVirtualAddress();
//			renderComponent.m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
//			renderComponent.m_indexBufferView.SizeInBytes = sizeof(materialComponent.m_indices[0]) * materialComponent.m_indices.size();
//
//			// Load the vertex shader.
//			Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
//			dxutils::ThrowIfFailed(D3DReadFileToBlob(materialComponent.m_vs.data(), &vertexShaderBlob));
//			// Load the pixel shader.
//			Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
//			dxutils::ThrowIfFailed(D3DReadFileToBlob(materialComponent.m_ps.data(), &pixelShaderBlob));
//
//			// Create a root signature.
//			D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
//			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
//			if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
//			{
//				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
//			}
//
//			// Allow input layout and deny unnecessary access to certain pipeline stages.
//			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
//				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
//				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
//				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
//				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
//				D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
//
//			// A single 32-bit constant root parameter that is used by the vertex shader.
//			CD3DX12_ROOT_PARAMETER1 rootParameters[1] = { };
//			rootParameters[0].InitAsConstants(
//				materialComponent.m_vsParamLayout.num32BitValues, 
//				materialComponent.m_vsParamLayout.shaderRegister, 
//				materialComponent.m_vsParamLayout.registerSpace,
//				materialComponent.m_vsParamLayout.visibility);
//
//			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
//			rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);
//			// Serialize the root signature.
//			Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
//			Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
//			dxutils::ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
//				featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
//
//			// Create the root signature.
//			dxutils::ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
//				rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&renderComponent.m_rootSignature)));
//
//			struct PipelineStateStream
//			{
//				CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
//				CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
//				CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
//				CD3DX12_PIPELINE_STATE_STREAM_VS VS;
//				CD3DX12_PIPELINE_STATE_STREAM_PS PS;
//				CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
//				CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
//			} pipelineStateStream;
//
//			D3D12_RT_FORMAT_ARRAY rtvFormats = {};
//			rtvFormats.NumRenderTargets = 1;
//			rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//
//			pipelineStateStream.pRootSignature = renderComponent.m_rootSignature.Get();
//			pipelineStateStream.InputLayout = { materialComponent.m_vsInputLayout.data(), (UINT)materialComponent.m_vsInputLayout.size() };
//			pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//			pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
//			pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
//			pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
//			pipelineStateStream.RTVFormats = rtvFormats;
//
//			D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
//			sizeof(PipelineStateStream), &pipelineStateStream
//			};
//			dxutils::ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&renderComponent.m_pipelineState)));
//
//			auto fenceValue = commandQueue->ExecuteCommandList(commandList);
//			commandQueue->WaitForFenceValue(fenceValue);
//		}
//	}
//};
//
//class InitializeTransformComponents : public ISystem
//{
//public:
//	void Execute() override
//	{
//		TransformComponentIterator* it = (TransformComponentIterator*)IComponent::CreateIterator(ComponentType::TransformComponent, 1);
//		TransformComponent* material = (TransformComponent*)it->At(0);
//		material->m_entityId = 0;
//		material->m_position = { 0.f, 0.f, 0.f, 1.f };
//		material->m_rotation = { 0.f, 1.f, 1.f, 0.f };
//		material->m_scale = { 1.f, 1.f, 1.f, 0.f };
//
//		ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::TransformComponent, it);
//	}
//};
//
//class InitializeEntitiesSystem : public ISystem
//{
//public:
//	void Execute() override
//	{
//		const ComponentsPool& componentsPool = ECSManager::GetInstance()->GetComponentsPool();
//		std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
//
//		for (size_t i = static_cast<size_t>(ComponentType::DX12MaterialComponent);
//			i < static_cast<size_t>(ComponentType::Size);
//			++i)
//		{
//			Iterator* it = componentsPool.GetComponents(static_cast<ComponentType>(i));
//			for (size_t componentIdx = 0; componentIdx < it->Num(); ++componentIdx)
//			{
//				IComponent* component = it->At(componentIdx);
//				
//				std::vector<Entity>::iterator entityWithThisId = std::find_if(entities.begin(), entities.end(),
//					[component](const Entity& a) { return a.m_entityId == component->m_entityId; });
//				if (entityWithThisId == entities.end())
//				{
//					Entity newEntity;
//					newEntity.m_entityId = component->m_entityId;
//					newEntity.m_components.insert({ component->GetComponentType(), component });
//					entities.push_back(newEntity);
//				}
//				else
//				{
//					entityWithThisId->m_components.insert({ component->GetComponentType(), component });
//				}
//			}
//		}
//	}
//};
//
//class RenderSystem : public ISystem
//{
//public:
//	void Execute() override
//	{
//		win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
//		const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
//		for (const Entity& entity : entities)
//		{
//			const DX12RenderComponent& renderable = *(DX12RenderComponent*)entity.m_components.at(ComponentType::DX12RenderComponent);
//			const TransformComponent& transform = *(TransformComponent*)entity.m_components.at(ComponentType::TransformComponent);
//
//			auto commandQueue = dx12Driver->GetDX12CommandQueue(win::DX12Driver::CommandQueueType::Direct);
//			auto commandList = commandQueue->GetD3D12CommandList();
//
//			auto backBuffer = dx12Driver->m_backBuffers[dx12Driver->m_currentBackBufferIndex];
//			auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(dx12Driver->m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), dx12Driver->m_currentBackBufferIndex, dx12Driver->m_RTVDescriptorSize);
//			auto dsv = dx12Driver->m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
//
//			// Clear the render targets.
//			{
//				dxutils::TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
//
//				float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//				dxutils::ClearRTV(commandList, rtv, color);
//				dxutils::ClearDSV(commandList, dsv);
//			}
//
//			commandList->SetPipelineState(renderable.m_pipelineState.Get());
//			commandList->SetGraphicsRootSignature(renderable.m_rootSignature.Get());
//
//			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//			commandList->IASetVertexBuffers(0, 1, &renderable.m_vertexBufferView);
//			commandList->IASetIndexBuffer(&renderable.m_indexBufferView);
//
//			commandList->RSSetViewports(1, &dx12Driver->m_viewport);
//			commandList->RSSetScissorRects(1, &dx12Driver->m_scissorRect);
//
//			commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
//
//			// Update the MVP matrix
//			const DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(transform.m_rotation.m_x, transform.m_rotation.m_y, transform.m_rotation.m_z, transform.m_rotation.m_w);
//			const DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, 0.f);
//			DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixMultiply(modelMatrix, dx12Driver->m_viewMatrix);
//			mvpMatrix = DirectX::XMMatrixMultiply(mvpMatrix, dx12Driver->m_projectionMatrix);
//			commandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / _countof(DirectX::XMMATRIX::r), &mvpMatrix, 0);
//
//			commandList->DrawIndexedInstanced(renderable.m_indexBufferView.SizeInBytes / sizeof(uint16_t), 1, 0, 0, 0);
//
//			// Present
//			{
//				dxutils::TransitionResource(commandList, backBuffer,
//					D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
//
//				dx12Driver->m_frameFenceValues[dx12Driver->m_currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
//
//				{
//					UINT syncInterval = dx12Driver->m_isVSync ? 1 : 0;
//					UINT presentFlags = dx12Driver->m_isTearingSupported && !dx12Driver->m_isVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
//					dxutils::ThrowIfFailed(dx12Driver->m_swapChain->Present(syncInterval, presentFlags));
//					dx12Driver->m_currentBackBufferIndex = dx12Driver->m_swapChain->GetCurrentBackBufferIndex();
//				}
//
//				commandQueue->WaitForFenceValue(dx12Driver->m_frameFenceValues[dx12Driver->m_currentBackBufferIndex]);
//			}
//		}
//
//		/*Iterator* renderComponentsIt = ECSManager::GetInstance()->GetComponentsPool().GetComponents(ComponentType::DX12RenderComponent);
//		DX12RenderComponent* renderComponents = (DX12RenderComponent*)renderComponentsIt->GetData();
//
//		win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
//		for (int i = 0; i < renderComponentsIt->Num(); ++i)
//		{
//			const DX12RenderComponent& renderable = renderComponents[i];
//
//			auto commandQueue = dx12Driver->GetDX12CommandQueue(win::DX12Driver::CommandQueueType::Direct);
//			auto commandList = commandQueue->GetD3D12CommandList();
//
//			auto backBuffer = dx12Driver->m_backBuffers[dx12Driver->m_currentBackBufferIndex];
//			auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(dx12Driver->m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), dx12Driver->m_currentBackBufferIndex, dx12Driver->m_RTVDescriptorSize);
//			auto dsv = dx12Driver->m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
//
//			// Clear the render targets.
//			{
//				dxutils::TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
//
//				float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//				dxutils::ClearRTV(commandList, rtv, color);
//				dxutils::ClearDSV(commandList, dsv);
//			}
//
//			commandList->SetPipelineState(renderable.m_pipelineState.Get());
//			commandList->SetGraphicsRootSignature(renderable.m_rootSignature.Get());
//
//			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//			commandList->IASetVertexBuffers(0, 1, &renderable.m_vertexBufferView);
//			commandList->IASetIndexBuffer(&renderable.m_indexBufferView);
//
//			commandList->RSSetViewports(1, &dx12Driver->m_viewport);
//			commandList->RSSetScissorRects(1, &dx12Driver->m_scissorRect);
//
//			commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
//
//			// Update the MVP matrix
//			DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixMultiply(dx12Driver->m_modelMatrix, dx12Driver->m_viewMatrix);
//			mvpMatrix = DirectX::XMMatrixMultiply(mvpMatrix, dx12Driver->m_projectionMatrix);
//			commandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / _countof(DirectX::XMMATRIX::r), &mvpMatrix, 0);
//
//			commandList->DrawIndexedInstanced(renderable.m_indexBufferView.SizeInBytes / sizeof(uint16_t), 1, 0, 0, 0);
//
//			// Present
//			{
//				dxutils::TransitionResource(commandList, backBuffer,
//					D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
//
//				dx12Driver->m_frameFenceValues[dx12Driver->m_currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
//
//				{
//					UINT syncInterval = dx12Driver->m_isVSync ? 1 : 0;
//					UINT presentFlags = dx12Driver->m_isTearingSupported && !dx12Driver->m_isVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
//					dxutils::ThrowIfFailed(dx12Driver->m_swapChain->Present(syncInterval, presentFlags));
//					dx12Driver->m_currentBackBufferIndex = dx12Driver->m_swapChain->GetCurrentBackBufferIndex();
//				}
//
//				commandQueue->WaitForFenceValue(dx12Driver->m_frameFenceValues[dx12Driver->m_currentBackBufferIndex]);
//			}
//		}*/
//	}
//};

}
}