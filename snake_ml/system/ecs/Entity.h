// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "IComponent.h"
#include <unordered_map>

namespace snakeml
{
namespace system
{

class Entity
{
public:
	uint32_t m_entityId;
	std::unordered_map<ComponentType, IComponent*> m_components;
};

}
}