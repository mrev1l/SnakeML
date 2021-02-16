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
	Size
};

#define REGISTER_TYPE(ObjectType) \
	class ObjectType##Iterator : public Iterator { \
	public: \
		ObjectType##Iterator(IComponent* data, size_t num) : Iterator(data, num) {} \
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
        virtual IComponent* Create() { \
            return new ObjectType(); \
        } \
        virtual IComponent* Create(size_t num) { \
            return new ObjectType[num]; \
        } \
		virtual Iterator* CreateIterator(size_t num) { \
			return new ObjectType##Iterator(new ObjectType[num](), num); \
		} \
}; \
static ObjectType##Factory global_##ObjectType##Factory; \

class IComponent;

class Iterator
{
public:
	Iterator(IComponent* data, size_t num) : m_data(data), m_count(num) {};
	IComponent* At(size_t idx)
	{
		return GetElement(idx);
	}

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
	virtual IComponent* Create() = 0;
	virtual IComponent* Create(size_t num) = 0;
	virtual Iterator* CreateIterator(size_t num) = 0;
};

class IComponent
{
public:
	virtual ComponentType GetComponentType() const = 0;

	static void RegisterFactory(ComponentType objType, Factory* objFactory)
	{
		factories.insert({ objType, objFactory });
	}

	static IComponent* Create(ComponentType objType)
	{
		return factories.at(objType)->Create();
	}

	static IComponent* Create(ComponentType objType, size_t num)
	{
		return factories.at(objType)->Create(num);
	}

	static Iterator* CreateIterator(ComponentType objType, size_t num)
	{
		return factories.at(objType)->CreateIterator(num);
	}

	uint32_t m_entityId;

private:
	inline static std::unordered_map<ComponentType, Factory*> factories;
};

}
}