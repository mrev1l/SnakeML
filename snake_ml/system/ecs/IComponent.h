// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "utils/patterns/castable.h"

namespace snakeml
{

enum class ComponentType : uint32_t
{
	MaterialComponent = 0,
	DX12RenderComponent,
	TransformComponent,
	CameraComponent,
	PhysicsComponent,
	MeshComponent,
	Size
};

#define REGISTER_TYPE(ObjectType) \
	class ObjectType##Iterator : public IteratorCastableImpl<ObjectType##Iterator> { \
	public: \
		ObjectType##Iterator(IComponent* data, size_t num) : IteratorCastableImpl<ObjectType##Iterator>(data, num) { \
		} \
		virtual ~ObjectType##Iterator() { \
			ObjectType* concreteArray = static_cast<ObjectType*>(m_data); \
			delete[] concreteArray; \
		} \
		\
		ObjectType& At(size_t idx) const { \
			ObjectType* concreteArray = static_cast<ObjectType*>(m_data); \
			return concreteArray[idx]; \
		} \
		\
		void Clear() override { \
			IComponent::DeleteIterator(At(0).GetComponentType(), this); \
		} \
		\
		ObjectType* begin() const { \
			return static_cast<ObjectType*>(m_data); \
		} \
		\
		ObjectType* end() const { \
			return static_cast<ObjectType*>(m_data) + m_count; \
		} \
	}; \
	\
	class ObjectType##Factory : public Factory { \
	public: \
		ObjectType##Factory() \
		{ \
			IComponent::RegisterFactory(ComponentType::##ObjectType, this); \
		} \
		virtual IComponent* Create() override { \
			return new ObjectType(); \
		} \
		virtual IComponent* Create(size_t num) override { \
			return new ObjectType[num]; \
		} \
		virtual Iterator* CreateIterator(size_t num) override { \
			return new ObjectType##Iterator(new ObjectType[num](), num); \
		} \
		virtual void DeleteIterator(Iterator* it) override { \
			ObjectType##Iterator* itToDelete = static_cast<ObjectType##Iterator*>(it); \
			delete itToDelete; \
		} \
}; \
static ObjectType##Factory global_##ObjectType##Factory; \

class IComponent;

class Iterator : public ICastable
{
public:
	Iterator(IComponent* data, size_t num) : m_data(data), m_count(num) { };
	virtual ~Iterator() = default;

	virtual void Clear() = 0;

	size_t Size() const { return m_count; }

protected:
	IComponent* m_data;
	size_t m_count;
};
REGISTER_CASTABLE_TYPE(Iterator);

class Factory
{
public:
	virtual ~Factory() = default;

	virtual IComponent* Create() = 0;
	virtual IComponent* Create(size_t num) = 0;
	virtual Iterator* CreateIterator(size_t num) = 0;
	virtual void DeleteIterator(Iterator* it) = 0;
};

class IComponent : public ICastable
{
public:
	virtual ~IComponent() = default;
	virtual ComponentType GetComponentType() const = 0;

	static void RegisterFactory(ComponentType objType, Factory* objFactory);
	static Iterator* CreateIterator(ComponentType objType, size_t num);
	static void DeleteIterator(ComponentType objType, Iterator* it);

	uint32_t m_entityId = -1;

private:
	inline static std::unordered_map<ComponentType, Factory*> factories;
};
REGISTER_CASTABLE_TYPE(IComponent);

}
