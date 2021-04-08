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

template<typename T>
class QuadTree
{
public:
	struct Rectangle
	{
		math::vector origin;
		math::vector halfDimensions;

		bool Contains(math::vector point) const;
		bool Intersects(Rectangle other) const;
	};

	struct Point
	{
		math::vector position;
		const T& userData;
	};

	QuadTree(Rectangle boundary);
	~QuadTree() = default;
	
	bool AddPoint(const Point& point);
	void GetPoints(Rectangle boundary, std::back_insert_iterator<std::vector<Point>> resIt) const;

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

	Rectangle									m_boundary;
	std::vector<Point>							m_points;
	std::array<std::unique_ptr<QuadTree>, 4u>	m_subTrees;
	bool										m_isSubDivided = false;

	static constexpr size_t s_capacity = 4;
};

void WIP_System::Update(double deltaTime)
{
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
	QuadTree<AABB> qt(QuadTree<AABB>::Rectangle{ {0.f, 0.f, 0.f}, {370.f, 370.f, 0.f} });
	for (size_t i = 0u; i < bodys->Size(); ++i)
	{
		const PhysicsComponent& body = *((PhysicsComponent*)bodys->At(i));

		qt.AddPoint(QuadTree<AABB>::Point{body.m_position, body.m_aabb});
	}

	// Test overlaps
	for (size_t i = 0u; i < bodys->Size(); ++i)
	{
		const PhysicsComponent& body = *((PhysicsComponent*)bodys->At(i));
		
		QuadTree<AABB>::Rectangle boundary = { body.m_position, { 720.f, 105.f, 0.f } };
		std::vector<QuadTree<AABB>::Point> points;

		qt.GetPoints(boundary, std::back_inserter(points));

		for (auto point : points)
		{
			if (
				point.userData.min.x == body.m_aabb.min.x &&
				point.userData.min.y == body.m_aabb.min.y &&
				point.userData.max.x == body.m_aabb.max.x &&
				point.userData.max.y == body.m_aabb.max.y)
			{
				return;
			}
			const AABB& a = body.m_aabb;
			const AABB& b = point.userData;

			float d1x = b.min.x - a.max.x;
			float d1y = b.min.y - a.max.y;
			float d2x = a.min.x - b.max.x;
			float d2y = a.min.y - b.max.y;

			if (d1x >= 0.f || d1y >= 0.f)
			{
				continue;
			}

			if (d2x >= 0.f || d2y >= 0.f)
			{
				continue;
			}

			s_update = false;
		}
	}
	/*for (size_t i = 0u; i < bodys->Size(); ++i)
	{
		for (size_t j = 0u; j < bodys->Size(); ++j)
		{
			if (i == j)
			{
				continue;
			}

			const PhysicsComponent& bodyA = *((PhysicsComponent*)bodys->At(i));
			const PhysicsComponent& bodyB = *((PhysicsComponent*)bodys->At(j));

			const AABB& a = bodyA.m_aabb;
			const AABB& b = bodyB.m_aabb;

			float d1x = b.min.x - a.max.x;
			float d1y = b.min.y - a.max.y;
			float d2x = a.min.x - b.max.x;
			float d2y = a.min.y - b.max.y;

			if (d1x >= 0.f || d1y >= 0.f)
			{
				continue;
			}

			if (d2x >= 0.f || d2y >= 0.f)
			{
				continue;
			}

			s_update = false;
		}
	}*/

	int stop = 45;
}

template<typename T>
QuadTree<T>::QuadTree(Rectangle boundary)
	: m_boundary(boundary)
{
	m_points.reserve(s_capacity);
}

template<typename T>
bool QuadTree<T>::AddPoint(const Point& point)
{
	if (!m_boundary.Contains(point.position))
	{
		return false;
	}

	if (m_points.size() < s_capacity)
	{
		m_points.emplace_back(point);
	}
	else
	{
		if (!m_isSubDivided)
		{
			SubDivide();
		}

		bool wasAdded = m_subTrees[NorthWest]->AddPoint(point);
		wasAdded |= m_subTrees[NorthEast]->AddPoint(point);
		wasAdded |= m_subTrees[SouthWest]->AddPoint(point);
		wasAdded |= m_subTrees[SouthEast]->AddPoint(point);

		ASSERT(wasAdded, "QuadTree node accepted the point that was not inserted.");
	}

	return true;
}

template<typename T>
void QuadTree<T>::GetPoints(QuadTree<T>::Rectangle boundary, std::back_insert_iterator<std::vector<Point>> resIt) const
{
	if (!m_boundary.Intersects(boundary))
	{
		return;
	}

	for (auto point : m_points)
	{
		if (boundary.Contains(point.position))
		{
			resIt = point;
		}
	}

	if (m_isSubDivided)
	{
		m_subTrees[NorthWest]->GetPoints(boundary, resIt);
		m_subTrees[NorthEast]->GetPoints(boundary, resIt);
		m_subTrees[SouthWest]->GetPoints(boundary, resIt);
		m_subTrees[SouthEast]->GetPoints(boundary, resIt);
	}
}

template<typename T>
void QuadTree<T>::SubDivide()
{
	ASSERT(!m_isSubDivided, "Trying to re-subdivide the QuadTree.");

	math::vector northwestOrigin = { m_boundary.origin.x - m_boundary.halfDimensions.x / 2, m_boundary.origin.y + m_boundary.halfDimensions.y / 2, 0.f };
	math::vector northeastOrigin = { m_boundary.origin.x + m_boundary.halfDimensions.x / 2, m_boundary.origin.y + m_boundary.halfDimensions.y / 2, 0.f };
	math::vector southwestOrigin = { m_boundary.origin.x - m_boundary.halfDimensions.x / 2, m_boundary.origin.y - m_boundary.halfDimensions.y / 2, 0.f };
	math::vector southeastOrigin = { m_boundary.origin.x + m_boundary.halfDimensions.x / 2, m_boundary.origin.y - m_boundary.halfDimensions.y / 2, 0.f };

	Rectangle northwest{ northwestOrigin, m_boundary.halfDimensions / 2};
	Rectangle northeast{ northeastOrigin, m_boundary.halfDimensions / 2};
	Rectangle southwest{ southwestOrigin, m_boundary.halfDimensions / 2};
	Rectangle southeast{ southeastOrigin, m_boundary.halfDimensions / 2};

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
bool QuadTree<T>::Rectangle::Contains(math::vector point) const
{
	return
		point.x >= origin.x - halfDimensions.x &&
		point.x <= origin.x + halfDimensions.x &&
		point.y >= origin.y - halfDimensions.y &&
		point.y <= origin.y + halfDimensions.y;
}

template<typename T>
bool QuadTree<T>::Rectangle::Intersects(Rectangle other) const
{
	float d1x = origin.x - halfDimensions.x - other.origin.x + other.halfDimensions.x;
	float d1y = origin.y - halfDimensions.y - other.origin.y + other.halfDimensions.y;
	float d2x = other.origin.x - other.halfDimensions.x - origin.x + halfDimensions.x;
	float d2y = other.origin.y - other.halfDimensions.y - origin.y + halfDimensions.y;

	bool test = !((d1x >= 0.f) && (d1y >= 0.f) && (d2x >= 0.f) && (d2y >= 0.f));
	if (test)
	{
		int stop = 345;
	}
	return !((d1x >= 0.f) && (d1y >= 0.f) && (d2x >= 0.f) && (d2y >= 0.f));
}

}
}