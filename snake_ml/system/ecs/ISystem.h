// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "IComponent.h"

namespace snakeml
{

enum class TickGroup
{
	PrePhysics,
	Physics,
	PostPhysics,
	Render
};

class ISystem : public ICastable
{
public:
	ISystem(std::vector<uint32_t> targetEntities = std::vector<uint32_t>()) : m_targetEntities(targetEntities), m_id(++s_globalSystemIdCounter) {}
	virtual ~ISystem() = 0 {};

	virtual void Execute() {} // TODO: private + friend class?
	virtual void Update(float deltaTime) {} // TODO: private + friend class?

	uint32_t m_id = 0;

protected:
	/// if empty - all the entities
	std::vector<uint32_t> m_targetEntities; // TODO : Abstract the interation over targets

private:
	// TODO deleted copy ctor
	// Add nocopyable interface
	static uint32_t s_globalSystemIdCounter;
};
REGISTER_CASTABLE_TYPE(ISystem);

}
