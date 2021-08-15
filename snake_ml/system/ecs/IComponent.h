// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include <unordered_map>

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
	class ObjectType##Iterator : public IteratorBaseImpl<ObjectType##Iterator> { \
	public: \
		ObjectType##Iterator(IComponent* data, size_t num) : IteratorBaseImpl<ObjectType##Iterator>(data, num) { \
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

class Iterator
{
public:
	Iterator(IComponent* data, size_t num) : m_data(data), m_count(num) { };
	virtual ~Iterator() = default;

	virtual void Clear() = 0;

	size_t Size() const { return m_count; }

	template<class T>
	T* As();

protected:
	virtual const std::type_info& GetTypeInfo() const = 0;

	IComponent* m_data;
	size_t m_count;
};

template<class T>
class IteratorBaseImpl : public Iterator
{
public:
	IteratorBaseImpl(IComponent* data, size_t num) : Iterator(data, num) { };
	virtual ~IteratorBaseImpl() = default;

	void Clear() override;

protected:
	const std::type_info& GetTypeInfo() const override;
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

	template<class T>
	T* As();

	static void RegisterFactory(ComponentType objType, Factory* objFactory);
	static Iterator* CreateIterator(ComponentType objType, size_t num);
	static void DeleteIterator(ComponentType objType, Iterator* it);

	uint32_t m_entityId = -1;

protected:
	virtual const std::type_info& GetTypeInfo() const = 0;

private:
	inline static std::unordered_map<ComponentType, Factory*> factories;
};

template<class T>
class ComponentBaseImpl : public IComponent
{
protected:
	const std::type_info& GetTypeInfo() const override;
	
};

#include "IComponent.inl"

}
