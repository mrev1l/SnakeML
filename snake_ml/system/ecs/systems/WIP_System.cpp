// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "WIP_System.h"

#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include <chrono>
#include <iterator>

namespace snakeml
{
namespace system
{

bool TestIntersection_SAT(const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices)
{
	std::vector<math::vector> axises;
	
	// TODO : remove code duplication

	// building axises off the first polygon edges
	for (size_t i = 0u; i < aVertices.size(); ++i)
	{
		const size_t j = (i + 1) % aVertices.size();
		math::vector edge = aVertices[j] - aVertices[i];
		// calculate perpendicular vector in 2D
		edge = { -edge.y, edge.x, 0.f };
		const math::vector axisCandidate = edge.getNormalized();

		// don't allow the same axis to occur multiple times
		if (std::find_if(axises.begin(), axises.end(), [axisCandidate](const math::vector& a)
			{
				float dot = axisCandidate.dot(a);
				return math::IsNearlyEqual(dot, 1.f, cosf(math::ConvertToRadians(1.f))) ||
					math::IsNearlyEqual(dot, -1.f, cosf(math::ConvertToRadians(1.f)));
			}) == axises.end())
		{
			axises.push_back(axisCandidate);
		}
	}

	// building axises off the second polygon edges
	for (size_t i = 0u; i < bVertices.size(); ++i)
	{
		const size_t j = (i + 1) % aVertices.size();
		math::vector edge = bVertices[j] - bVertices[i];
		// calculate perpendicular vector in 2D
		edge = { -edge.y, edge.x, 0.f };
		
		const math::vector axisCandidate = edge.getNormalized();

		// don't allow the same axis to occur multiple times
		if (std::find_if(axises.begin(), axises.end(), [axisCandidate](const math::vector& a)
			{
				float dot = axisCandidate.dot(a);
				return math::IsNearlyEqual(dot, 1.f, cosf(math::ConvertToRadians(1.f))) ||
					math::IsNearlyEqual(dot, -1.f, cosf(math::ConvertToRadians(1.f)));
			}) == axises.end())
		{
			axises.push_back(axisCandidate);
		}
	}

	// TODO : remove code duplication

	for (const math::vector& axis : axises)
	{
		float aProjectionMin = FLT_MAX, aProjectionMax = -FLT_MAX, bProjectionMin = FLT_MAX, bProjectionMax = -FLT_MAX;

		for (const math::vector& aVertex : aVertices)
		{
			float projection = aVertex.dot(axis);
			aProjectionMin = std::min(aProjectionMin, projection);
			aProjectionMax = std::max(aProjectionMax, projection);
		}

		for (const math::vector& bVertex : bVertices)
		{
			float projection = bVertex.dot(axis);
			bProjectionMin = std::min(bProjectionMin, projection);
			bProjectionMax = std::max(bProjectionMax, projection);
		}

		if (
			aProjectionMax <= bProjectionMin || // a is before b
			aProjectionMin >= bProjectionMax)	// a is after b
		{
			return false;
		}
	}

	return true; // no separating axis found
}

bool TestIntersection_AABB_AABB(const AABB& a, const AABB& b)
{
	const bool isABehindB = a.min.x >= b.max.x;
	const bool isABeforeB = a.max.x <= b.min.x;
	const bool isAAboveA = a.min.y >= b.max.y;
	const bool isABelowB = a.max.y <= b.min.y;

	return !(isABehindB || isABeforeB || isAAboveA || isABelowB);
}

template<typename T>
class QuadTree
{
public:
	struct Rectangle
	{
		math::vector origin;
		math::vector halfDimensions;

		bool Intersects(Rectangle other) const;
	};

	struct Object
	{
		Rectangle shape;
		const T& userData;
	};

	QuadTree(Rectangle boundary);
	~QuadTree() = default;
	
	bool AddObject(const Object& object);
	void GetObjects(Rectangle boundary, std::vector<const Object const *>& _outResult) const;

private:
	enum
	{
		NorthWest = 0,
		NorthEast,
		SouthWest,
		SouthEast
	};

	QuadTree(const QuadTree& other) = delete;
	QuadTree(QuadTree&& other) = delete;
	QuadTree& operator=(const QuadTree& other) = delete;
	QuadTree& operator=(QuadTree&& other) = delete;

	void SubDivide();

	static void TryPushObject(const Object& obj, std::vector<const Object const*>& _outResult);

	Rectangle									m_boundary;
	std::vector<Object>							m_objects;
	std::array<std::unique_ptr<QuadTree>, 4u>	m_subTrees;
	bool										m_isSubDivided = false;

	static constexpr size_t s_capacity = 4;
};

void WIP_System::Update(double deltaTime)
{
	{
		// sat test
		std::vector<math::vector> polygonA = {
			{ 1.f, 1.f, 0.f },
			{ 1.f, 3.f, 0.f },
			{ 3.f, 1.f, 0.f }
		};

		std::vector<math::vector> polygonB = {
			{ 1.f, 1.f, 0.f },
			{ 3.f, 1.f, 0.f },
			{ 3.f, 3.f, 0.f }
		};

		bool result = TestIntersection_SAT(polygonA, polygonB);
	}

	static bool s_update = true;

	const uint32_t entityIdToUpdate = 0;
	constexpr float scale = 0.1f;
	
	const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
	const std::vector<Entity>::const_iterator entityIt = std::find_if(entities.begin(), entities.end(),
		[entityIdToUpdate](const Entity& a) {return a.m_entityId == entityIdToUpdate; });
	if (entityIt != entities.end())
	{
		PhysicsComponent& body = *(PhysicsComponent*)entityIt->m_components.at(ComponentType::PhysicsComponent);

		// Linear movement integration
		if (s_update)
		{
			const math::vector force = { 1.f, -1.f, 0.f };
			math::vector accDelta = (force / body.m_shape.m_mass) * deltaTime;
			body.m_acceleration += accDelta;
			body.m_velocity += body.m_acceleration * deltaTime;
			body.m_position += body.m_velocity * deltaTime;

			// Angular movement integration
			const math::vector torque = math::vector::forward * 1000.f;
			body.m_angularAcceleration += (torque / body.m_shape.m_momentOfInertia) * deltaTime;
			body.m_angularVelocity += body.m_angularAcceleration * deltaTime;
			body.m_rotation += body.m_angularVelocity * deltaTime;

			TransformComponent& transform = *(TransformComponent*)(entityIt->m_components.at(ComponentType::TransformComponent));
			transform.m_scale.x = scale;
			transform.m_scale.y = scale;
			transform.m_position = body.m_position;
			transform.m_rotation = body.m_rotation;
		}
	}

	PhysicsComponentIterator* bodys = (PhysicsComponentIterator*)ECSManager::GetInstance()->GetComponentsPool().GetComponents(ComponentType::PhysicsComponent);
	// Update AABBs
	for (size_t i = 0u; i < bodys->Size(); ++i)
	{
		PhysicsComponent& body = *(PhysicsComponent*)bodys->At(i);

		std::array<math::vector, 4> boundingBox =
		{
			math::vector{/*body.m_position.x*/ - body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ - body.m_shape.m_dimensions.y / 2.f, 0.f},
			math::vector{/*body.m_position.x*/ - body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ + body.m_shape.m_dimensions.y / 2.f, 0.f},
			math::vector{/*body.m_position.x*/ + body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ - body.m_shape.m_dimensions.y / 2.f, 0.f},
			math::vector{/*body.m_position.x*/ + body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ + body.m_shape.m_dimensions.y / 2.f, 0.f}
		};

		const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(body.m_rotation.y), math::ConvertToRadians(body.m_rotation.x), math::ConvertToRadians(body.m_rotation.z));
		const math::matrix translationMatrix = math::TranslationMatrix(body.m_position.x, body.m_position.y, body.m_position.z);

		const math::matrix transformMatrix = rotationMatrix * translationMatrix;
		
		for (auto& vertex : boundingBox)
		{
			vertex = transformMatrix * vertex;
		}

		body.m_aabb = { {FLT_MAX, FLT_MAX, 0.f}, {-FLT_MAX, -FLT_MAX, 0.f} };
		for (auto vertex : boundingBox)
		{
			body.m_aabb.min.x = std::min(body.m_aabb.min.x, vertex.x);
			body.m_aabb.min.y = std::min(body.m_aabb.min.y, vertex.y);
			body.m_aabb.max.x = std::max(body.m_aabb.max.x, vertex.x);
			body.m_aabb.max.y = std::max(body.m_aabb.max.y, vertex.y);
		}
	}

	// Populate QuadTree
	QuadTree<PhysicsComponent> qt(QuadTree<PhysicsComponent>::Rectangle{ {0.f, 0.f, 0.f}, {370.f, 370.f, 0.f} });
	for (size_t i = 0u; i < bodys->Size(); ++i)
	{
		const PhysicsComponent& body = *((PhysicsComponent*)bodys->At(i));

		qt.AddObject(QuadTree<PhysicsComponent>::Object{ {body.m_position, {(body.m_aabb.max - body.m_aabb.min) / 2.f}}, body });
	}

	// Broad Phase
	std::vector<std::pair<const PhysicsComponent&, const PhysicsComponent&>> narrowPhasePairs;
	for (size_t i = 0u; i < bodys->Size(); ++i)
	{
		const PhysicsComponent& body = *((PhysicsComponent*)bodys->At(i));
		if (!body.m_isDynamic)
		{
			continue;
		}
		
		QuadTree<PhysicsComponent>::Rectangle boundary = { body.m_position, body.m_aabb.max - body.m_aabb.min };
		std::vector<const QuadTree<PhysicsComponent>::Object const*> objects;

		qt.GetObjects(boundary, objects);

		for (auto object : objects)
		{
			if (&object->userData == &body)
			{
				continue;
			}
			const AABB& a = body.m_aabb;
			const AABB& b = object->userData.m_aabb;

			const bool isIntersecting = TestIntersection_AABB_AABB(a, b);
			
			s_update = !isIntersecting;
			narrowPhasePairs.push_back({ body, object->userData });
		}
	}
	
	// Narrow Phase
	for (const std::pair<const PhysicsComponent&, const PhysicsComponent&>& pair : narrowPhasePairs)
	{
		//pair.first.
	}
	int stop = 45;
}

template<typename T>
QuadTree<T>::QuadTree(Rectangle boundary)
	: m_boundary(boundary)
{
	m_objects.reserve(s_capacity);
}

template<typename T>
bool QuadTree<T>::AddObject(const Object& point)
{
	if (!m_boundary.Intersects(point.shape))
	{
		return false;
	}

	if (m_objects.size() < s_capacity)
	{
		m_objects.emplace_back(point);
	}
	else
	{
		if (!m_isSubDivided)
		{
			SubDivide();
		}

		bool wasAdded = m_subTrees[NorthWest]->AddObject(point);
		wasAdded |= m_subTrees[NorthEast]->AddObject(point);
		wasAdded |= m_subTrees[SouthWest]->AddObject(point);
		wasAdded |= m_subTrees[SouthEast]->AddObject(point);

		ASSERT(wasAdded, "QuadTree node accepted the point that was not inserted.");
	}

	return true;
}

template<typename T>
void QuadTree<T>::GetObjects(QuadTree<T>::Rectangle boundary, std::vector<const Object const*>& _outResult) const
{
	if (!m_boundary.Intersects(boundary))
	{
		return;
	}

	for (const auto& object : m_objects)
	{
		if (boundary.Intersects(object.shape))
		{
			TryPushObject(object, _outResult);
		}
	}

	if (m_isSubDivided)
	{
		m_subTrees[NorthWest]->GetObjects(boundary, _outResult);
		m_subTrees[NorthEast]->GetObjects(boundary, _outResult);
		m_subTrees[SouthWest]->GetObjects(boundary, _outResult);
		m_subTrees[SouthEast]->GetObjects(boundary, _outResult);
	}
}

template<typename T>
void QuadTree<T>::SubDivide()
{
	ASSERT(!m_isSubDivided, "Trying to re-subdivide the QuadTree.");

	math::vector northwestOrigin = { m_boundary.origin.x - m_boundary.halfDimensions.x / 2.f, m_boundary.origin.y + m_boundary.halfDimensions.y / 2.f, 0.f };
	math::vector northeastOrigin = { m_boundary.origin.x + m_boundary.halfDimensions.x / 2.f, m_boundary.origin.y + m_boundary.halfDimensions.y / 2.f, 0.f };
	math::vector southwestOrigin = { m_boundary.origin.x - m_boundary.halfDimensions.x / 2.f, m_boundary.origin.y - m_boundary.halfDimensions.y / 2.f, 0.f };
	math::vector southeastOrigin = { m_boundary.origin.x + m_boundary.halfDimensions.x / 2.f, m_boundary.origin.y - m_boundary.halfDimensions.y / 2.f, 0.f };

	Rectangle northwest{ northwestOrigin, m_boundary.halfDimensions / 2.f };
	Rectangle northeast{ northeastOrigin, m_boundary.halfDimensions / 2.f };
	Rectangle southwest{ southwestOrigin, m_boundary.halfDimensions / 2.f };
	Rectangle southeast{ southeastOrigin, m_boundary.halfDimensions / 2.f };

	m_subTrees =
	{
		std::make_unique<QuadTree>(northwest),
		std::make_unique<QuadTree>(northeast),
		std::make_unique<QuadTree>(southwest),
		std::make_unique<QuadTree>(southeast)
	};

	m_isSubDivided = true;
}

template<typename T>
void QuadTree<T>::TryPushObject(const Object& obj, std::vector<const Object const*>& _outResult)
{
	// TODO : indenting
	if (std::find_if(_outResult.begin(), _outResult.end(), 
		[obj](const Object* a)
		{
			return a == &obj;
		}) != _outResult.end())
	{
		return;
	}
	_outResult.push_back(&obj);
}

template<typename T>
bool QuadTree<T>::Rectangle::Intersects(Rectangle other) const
{
	bool result = TestIntersection_AABB_AABB(
		{ origin - halfDimensions, origin + halfDimensions },
		{ other.origin - other.halfDimensions, other.origin + other.halfDimensions });

	return result;
}

}
}