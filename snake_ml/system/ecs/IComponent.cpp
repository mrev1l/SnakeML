// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "IComponent.h"

namespace snakeml
{
namespace system
{

void IComponent::RegisterFactory(ComponentType objType, Factory* objFactory)
{
	factories.insert({ objType, objFactory });
}

IComponent* IComponent::Create(ComponentType objType)
{
	return factories.at(objType)->Create();
}

IComponent* IComponent::Create(ComponentType objType, size_t num)
{
	return factories.at(objType)->Create(num);
}

Iterator* IComponent::CreateIterator(ComponentType objType, size_t num)
{
	return factories.at(objType)->CreateIterator(num);
}

void IComponent::DeleteIterator(ComponentType objType, Iterator* it)
{
	factories.at(objType)->DeleteIterator(it);
}

}
}

