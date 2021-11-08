// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "WIP_System.h"

#include "system/ecs/components/EntityControllerComponent.h"
#include "system/ecs/components/ChildControllerComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include <chrono>
#include <iterator>

namespace snakeml
{
namespace wip
{

void WIP_System::Update(float deltaTime)
{
	ECSManager::GetInstance()->UnscheduleSystem(this);
}

}
}
