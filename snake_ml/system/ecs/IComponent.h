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
	class ObjectType##Iterator : public Iterator { \
	public: \
		ObjectType##Iterator(IComponent* data, size_t num) : Iterator(data, num) { \
			Iterator::RegisterCastTableEntry(ComponentType::##ObjectType, typeid(ObjectType##Iterator)); \
		} \
		virtual ~ObjectType##Iterator() { \
			ObjectType* concreteArray = (ObjectType*)m_data; \
			delete[] concreteArray; \
		} \
		\
		ObjectType& At(size_t idx) const { \
			ObjectType* concreteArray = (ObjectType*)m_data; \
			return concreteArray[idx]; \
		} \
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
			IComponent::RegisterCastTableEntry(ComponentType::##ObjectType, typeid(ObjectType)); \
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
	Iterator(IComponent* data, size_t num) : m_data(data), m_count(num) { };
	virtual ~Iterator() = default;

	IComponent* GetInterfacePtr(size_t idx) { return GetElement(idx); }
	size_t Size() const { return m_count; }

	template<class T>
	T* As();

protected:
	virtual IComponent* GetElement(size_t idx) = 0;
	static void RegisterCastTableEntry(ComponentType type, const std::type_info& typeInfo);

	IComponent* m_data;
	size_t m_count;

private:
	inline static std::unordered_map<ComponentType, const std::type_info&> s_castTable;
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
	static void RegisterCastTableEntry(ComponentType type, const std::type_info& typeInfo);

	uint32_t m_entityId = -1;

private:
	inline static std::unordered_map<ComponentType, Factory*> factories;
	inline static std::unordered_map<ComponentType, const std::type_info&> s_castTable;
};

template<class T>
inline T* Iterator::As()
{
	ASSERT(m_count, "[IComponent] : Missconfigured component.")
	ASSERT(s_castTable.contains(m_data->GetComponentType()), "[IComponent] : Missconfigured component.")
	const std::type_info& tInfo = typeid(T);
	const std::type_info& thisTypeInfo = s_castTable.at(m_data->GetComponentType());
	if (thisTypeInfo.hash_code() == tInfo.hash_code())
	{
		return (T*)this;
	}
	return nullptr;
}

template<class T>
inline T* IComponent::As()
{
	ASSERT(s_castTable.contains(GetComponentType()), "[IComponent] : Missconfigured component.");
	const std::type_info& tInfo = typeid(T);
	const std::type_info& thisTypeInfo = s_castTable.at(GetComponentType());
	if (thisTypeInfo.hash_code() == tInfo.hash_code())
	{
		return (T*)this;
	}
	return nullptr;
}

}
