// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class TransformComponent : public IComponentCastableImpl<TransformComponent>
{
public:
	virtual ~TransformComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::TransformComponent; }

	vector m_position;
	vector m_rotation;
	vector m_scale;
};

class TransformComponentConstructionVisitor : public ConstructionVisitor
{
public:
	TransformComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() override { return ComponentType::TransformComponent; }

	void Visit(Iterator* it, Entity& entity) override;

private:
	static constexpr const char* k_positionValueName	= "position";
	static constexpr const char* k_rotationValueName	= "rotation";
	static constexpr const char* k_scaleValueName		= "scale";
};

REGISTER_COMPONENT(TransformComponent);

//class TransformComponentIterator : public IteratorCastableImpl<TransformComponentIterator>{
//public:
//	TransformComponentIterator(size_t capacity) : IteratorCastableImpl<TransformComponentIterator>(capacity) {
//		m_data = AllocTraits::allocate(m_allocator, capacity);
//		m_begin = static_cast<TransformComponent*>(m_data);
//	}		 
//	virtual ~TransformComponentIterator() {
//		for (size_t i = 0; i < m_size; ++i) {
//			AllocTraits::destroy(m_allocator, m_begin + i); 
//		}
//		AllocTraits::deallocate(m_allocator, m_begin, m_capacity);
//	}
//	
//	TransformComponent & Add() {
//		if (m_size == m_capacity) {
//			TransformComponent d;
//			std::vector<TransformComponent> test;
//			test.emplace_back();
//			test.shrink_to_fit();
//			test.emplace_back(d);
//			TransformComponent* t = test[0].As<TransformComponent>();
//			Reallocate(m_capacity * 2u);
//			for (size_t i = 0; i < m_size; ++i) {
//				Entity& entity = ECSManager::GetInstance()->GetEntity(m_begin[i].m_entityId); 
//				{
//					//TransformComponent* ptr = m_begin[i].As<TransformComponent>();
//					entity.m_components[ComponentType::TransformComponent] = &m_begin[i];
//					//entity.m_components.insert({ ComponentType::TransformComponent, &m_begin[i] });
//					int stop = 34;
//				}
//				int stop = 34;
//			}
//		}
//	
//		AllocTraits::construct(m_allocator, m_begin + m_size); 
//		++m_size; 
//	
//		return m_begin[m_size - 1]; 
//	} 
//	
//	TransformComponent & At(size_t idx) const {
//		ASSERT(idx < m_size, "[Iterator] : Trying to access object beyond container size.");
//		return m_begin[idx]; 
//	} 
//	
//	void Clear() override {
//		IComponent::DeleteIterator(ComponentType::TransformComponent, this); 
//	} 
//	
//	TransformComponent * begin() const {
//		return m_begin; 
//	}
//
//	TransformComponent * end() const {
//		return m_begin + m_size; 
//	} 
//	
//	ComponentType GetComponentType() override {
//		return ComponentType::TransformComponent; 
//	} 
//	
//	void Accept(const std::unique_ptr<ConstructionVisitor>&v, Entity & entity) override {
//		if (v->GetReceiverType() == ComponentType::TransformComponent) {
//			v->Visit(this, entity); 
//			return;
//		} 
//		ASSERT(false, "[Iterator::Accept] : got a suspicious visitor."); 
//	} 
//private: 
//	using Alloc = std::allocator<TransformComponent>; 
//	using AllocTraits = std::allocator_traits<Alloc>; 
//	
//	void Reallocate(size_t newCapacity) {
//		TransformComponent* tempIt = AllocTraits::allocate(m_allocator, newCapacity); 
//		// not working std::copy(m_begin, m_begin + m_capacity, tempIt); 
//		// working errno_t res =  memcpy_s(tempIt, m_capacity * sizeof(TransformComponent), m_begin, m_capacity * sizeof(TransformComponent));
//		
//		for (size_t i = 0; i < m_capacity; ++i)
//		{
//			AllocTraits::construct(m_allocator, tempIt + i, std::move(m_begin[i]));
//			AllocTraits::destroy(m_allocator, m_begin + i);
//		}
//
//		AllocTraits::deallocate(m_allocator, m_begin, m_capacity); 
//		
//		m_data = tempIt; 
//		m_begin = static_cast<TransformComponent*>(m_data); 
//		m_capacity = newCapacity; 
//	} 
//
//	Alloc m_allocator; 
//	TransformComponent* m_begin = nullptr; 
//}; 
//
//class TransformComponentFactory : public Factory{ 
//public: 
//	TransformComponentFactory() 
//	{ 
//		IComponent::RegisterFactory(ComponentType::TransformComponent, this); 
//	} 
//	virtual Iterator * CreateIterator(size_t capacity) override {
//		TransformComponentIterator* it = new TransformComponentIterator(capacity); 
//		ECSManager::GetInstance()->InsertComponents<TransformComponentIterator>(it); 
//		return it; 
//	} 
//	virtual std::unique_ptr<ConstructionVisitor> CreateIteratorConstructionVisitor(const rapidjson::Value & json) override {
//		return std::make_unique<TransformComponentConstructionVisitor>(json); 
//	} 
//	virtual void DeleteIterator(Iterator * it) override {
//		TransformComponentIterator* itToDelete = static_cast<TransformComponentIterator*>(it); 
//		delete itToDelete; 
//	} 
//}; 
//static TransformComponentFactory global_TransformComponentFactory; 

}
