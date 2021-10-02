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
	DebugRenderComponent,
	EntityControllerComponent,
	InputDataComponent,
	ConsumableComponent,

	Size
};

#define REGISTER_TYPE(ObjectType) \
	class ObjectType##Iterator : public IteratorCastableImpl<ObjectType##Iterator> { \
	public: \
		ObjectType##Iterator(size_t capacity) : IteratorCastableImpl<ObjectType##Iterator>(capacity) { \
			m_data = AllocTraits::allocate(m_allocator, capacity); \
			m_begin = static_cast<ObjectType*>(m_data); \
		} \
		virtual ~ObjectType##Iterator() { \
			for(size_t i = 0; i < m_size; ++i) { \
				AllocTraits::destroy(m_allocator, m_begin + i); \
			} \
			AllocTraits::deallocate(m_allocator, m_begin, m_capacity); \
		} \
		\
		ObjectType& Add() { \
			if (m_size == m_capacity) { \
				Reallocate(m_capacity * 2u); \
			} \
			\
			AllocTraits::construct(m_allocator, m_begin + m_size); \
			++m_size; \
			\
			return m_begin[m_size - 1]; \
		} \
		\
		ObjectType& At(size_t idx) const { \
			ASSERT(idx < m_size, "[Iterator] : Trying to access object beyond container size.") \
			return m_begin[idx]; \
		} \
		\
		void Clear() override { \
			IComponent::DeleteIterator(At(0).GetComponentType(), this); \
		} \
		\
		ObjectType* begin() const { \
			return m_begin; \
		} \
		\
		ObjectType* end() const { \
			return m_begin + m_size; \
		} \
	private: \
		using Alloc			= std::allocator<ObjectType>; \
		using AllocTraits	= std::allocator_traits<Alloc>; \
		\
		void Reallocate(size_t newCapacity) { \
			ObjectType* tempIt = AllocTraits::allocate(m_allocator, newCapacity); \
			std::copy(m_begin, m_begin + m_capacity, tempIt); \
			\
			AllocTraits::deallocate(m_allocator, m_begin, m_capacity); \
			\
			m_data = tempIt; \
			m_begin = static_cast<ObjectType*>(m_data); \
			m_capacity = newCapacity; \
		} \
		\
		Alloc m_allocator; \
		ObjectType* m_begin = nullptr; \
	}; \
	\
	class ObjectType##Factory : public Factory { \
	public: \
		ObjectType##Factory() \
		{ \
			IComponent::RegisterFactory(ComponentType::##ObjectType, this); \
		} \
		virtual Iterator* CreateIterator(size_t capacity) override { \
			return new ObjectType##Iterator(capacity); \
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
	Iterator(size_t capacity) : m_size(0), m_capacity(capacity) { };
	virtual ~Iterator() = default;

	virtual void Clear() = 0;

	size_t Size() const { return m_size; }

	static constexpr size_t k_defaultCapacity = 64u; // TODO : use

protected:
	IComponent*	m_data = nullptr;
	size_t		m_size = 0;
	size_t		m_capacity = 0;
};
REGISTER_CASTABLE_TYPE(Iterator);

class Factory
{
public:
	virtual ~Factory() = default;

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
