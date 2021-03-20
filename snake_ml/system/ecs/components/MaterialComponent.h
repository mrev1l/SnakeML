// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/IComponent.h"

#include "utils/types/arithmetic.h"

namespace snakeml
{
namespace system
{

class MaterialComponent : public IComponent
{
public:
	enum class InputLayoutEntries
	{
		Position,
		Color,
		UV,

		Count
	};

	virtual ~MaterialComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::MaterialComponent; }

	std::vector<std::pair<types::vec3<float>, types::vec2<float>>> m_vertices;
	std::vector<uint16_t> m_indices;
	std::wstring m_vs;
	std::wstring m_ps;
	std::vector<InputLayoutEntries> m_inputLayoutEntries;
	std::wstring m_texturePath;
};
REGISTER_TYPE(MaterialComponent);

}
}