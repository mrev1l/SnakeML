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
	virtual void Execute() = 0; // private + friend class?
};

}
}