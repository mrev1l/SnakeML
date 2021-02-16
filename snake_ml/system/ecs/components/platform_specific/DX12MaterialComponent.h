// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "lib_includes/directX_includes.h"
#include "system/ecs/IComponent.h"
#include "utils/snake_math.h"


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

	ComponentType GetComponentType() const override { return ComponentType::DX12MaterialComponent; }

	std::vector<std::pair<math::vec3<float>, math::vec3<float>>> m_vertices;
	std::vector<uint16_t> m_indices;
	std::wstring m_vs;
	std::wstring m_ps;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_vsInputLayout;
	VSParamLayout m_vsParamLayout;
};
REGISTER_TYPE(DX12MaterialComponent);

}
}