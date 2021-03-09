// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include <unordered_map>

namespace snakeml
{
namespace system
{

enum class ComponentType : uint32_t
{
	DX12MaterialComponent = 0,
	DX12RenderComponent,
	TransformComponent,
	CameraComponent,
	Size
};

#define REGISTER_TYPE(ObjectType) \
	class ObjectType##Iterator : public Iterator { \
	public: \
		ObjectType##Iterator(IComponent* data, size_t num) : Iterator(data, num) {} \
		virtual ~ObjectType##Iterator() { \
			ObjectType* concreteArray = (ObjectType*)m_data; \
			delete[] concreteArray; \
		} \
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
			ObjectType##Iterator* itToDelete = (ObjectType##Iterator*)it; \
			delete itToDelete; \
		} \
}; \
static ObjectType##Factory global_##ObjectType##Factory; \

class IComponent;

class Iterator
{
public:
	Iterator(IComponent* data, size_t num) : m_data(data), m_count(num) {};
	virtual ~Iterator() = default;

	IComponent* At(size_t idx) { return GetElement(idx); }

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
	virtual ~Factory() = default;

	virtual IComponent* Create() = 0;
	virtual IComponent* Create(size_t num) = 0;
	virtual Iterator* CreateIterator(size_t num) = 0;
	virtual void DeleteIterator(Iterator* it) = 0;
};

class IComponent
{
public:
	virtual ~IComponent() = default;
	virtual ComponentType GetComponentType() const = 0;

	static void RegisterFactory(ComponentType objType, Factory* objFactory);

	static IComponent* Create(ComponentType objType);
	static IComponent* Create(ComponentType objType, size_t num);

	static Iterator* CreateIterator(ComponentType objType, size_t num);
	static void DeleteIterator(ComponentType objType, Iterator* it);

	uint32_t m_entityId;

private:
	inline static std::unordered_map<ComponentType, Factory*> factories;
};

}
}