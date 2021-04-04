// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "IComponent.h"

namespace snakeml
{
namespace system
{

class ISystem
{
public:
	virtual ~ISystem() = 0 {};
	virtual void Execute() {} // TODO: private + friend class?
	virtual void Update(double deltaTime) {} // TODO: private + friend class?
};

}
}