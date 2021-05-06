// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "WIP_System.h"

#include "system/ecs/components/MeshComponent.h"
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

math::vector GJK_SupportFunction(const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices, const math::vector& direction)
{
	const math::vector& aDirection = direction;
	std::pair<math::vector, float> furthestPointA = { math::vector::zero, -FLT_MAX };
	for (const math::vector& v : aVertices)
	{
		const float dot = aDirection.dot(v);
		if (dot > furthestPointA.second)
		{
			furthestPointA = { v, dot };
		}
	}

	const math::vector& bDirection = -direction;
	std::pair<math::vector, float> furthestPointB = { math::vector::zero, -FLT_MAX };
	for (const math::vector& v : bVertices)
	{
		const float dot = bDirection.dot(v);
		if (dot > furthestPointB.second)
		{
			furthestPointB = { v, dot };
		}
	}

	return furthestPointA.first - furthestPointB.first;
}

math::vector triple_product(const math::vector& a, const math::vector& b, const math::vector& c)
{
	// https://en.wikipedia.org/wiki/Triple_product#Vector_triple_product
	//return b * a.dot(c) - c * (a.dot(b));
	return a.cross(b).cross(c);
}

math::vector perpendicular2d(const math::vector& a)
{
	return { -a.y, a.x, 0.f };
}

bool ShareAxis(const math::vector& a, const math::vector& b)
{
	const float dot = a.getNormalized().dot(b.getNormalized());
	return math::IsNearlyEqual(std::abs(dot), 1.f);
}

bool GJK_HandleSimplex_LineCase(const std::vector<math::vector>& simplex, math::vector& direction)
{
	const math::vector& a = simplex[1];
	const math::vector& b = simplex[0];
	const math::vector ab = b - a;
	const math::vector ao = math::vector::zero - a;

	const bool doShareAxis = ShareAxis(ab, ao);
	////const math::vector abPerp = triple_product(ab, ao, ab);
	//// calculate perpendicular vector in 2D
	//const math::vector abPerp =  { -ab.y, ab.x, 0.f };
	const math::vector triProduct = triple_product(ab, ao, ab);
	
	math::vector abPerp;// = doShareAxis ? perpendicular2d(ab) : triple_product(ab, ao, ab);
	if (math::IsNearlyZero(triProduct.length()))
	{
		abPerp = perpendicular2d(ab);
	}
	else
	{
		abPerp = triProduct;
	}

	direction = abPerp.getNormalized();
	return false;
}

bool GJK_HandleSimplex_TriangleCase(std::vector<math::vector>& simplex, math::vector& direction)
{
	const math::vector& a = simplex[2];
	const math::vector& b = simplex[1];
	const math::vector& c = simplex[0];
	const math::vector ab = b - a;
	const math::vector ac = c - a;
	const math::vector ao = math::vector::zero - a;
	const bool doShareAxis = ShareAxis(ab, ac);
	/*const math::vector abPerp = triple_product(ac, ab, ab);
	const math::vector acPerp = triple_product(ab, ac, ac);
	const math::vector abPerp1 = { -ab.y, ab.x, 0.f };
	const math::vector acPerp1 = { -ac.y, ac.x, 0.f };*/
	const math::vector abTriProduct = triple_product(ac, ab, ab);
	const math::vector acTriProduct = triple_product(ab, ac, ac);
	const math::vector abPerp = math::IsNearlyZero(abTriProduct.length()) ? perpendicular2d(ab) : abTriProduct;// doShareAxis ? perpendicular2d(ab) : triple_product(ac, ab, ab);
	const math::vector acPerp = math::IsNearlyZero(acTriProduct.length()) ? perpendicular2d(ac) : acTriProduct;// doShareAxis ? perpendicular2d(ac) : triple_product(ab, ac, ac);
	if (abPerp.dot(ao) > 0.f)
	{
		simplex.erase(simplex.begin());
		direction = abPerp.getNormalized();
		return false;
	}
	else if (acPerp.dot(ao) > 0.f)
	{
		simplex.erase(simplex.begin() + 1);
		direction = acPerp.getNormalized();
		return false;
	}
	return true;
}

bool GJK_HandleSimplex(std::vector<math::vector>& simplex, math::vector& direction)
{
	if (simplex.size() == 2)
	{
		return GJK_HandleSimplex_LineCase(simplex, direction);
	}
	ASSERT(simplex.size() == 3, "Error in GJK");
	return GJK_HandleSimplex_TriangleCase(simplex, direction);
}

struct Intersection
{
	bool areIntersecting = false;
	float penetrationDepth = 0.f;
	math::vector penetrationVector = math::vector::zero;
};
Intersection TestIntersection_GJK(const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices, const math::vector& aCenter, const math::vector& bCenter)
{
	// https://www.youtube.com/watch?v=ajv46BSqcK4
	math::vector direction = (bCenter - aCenter).getNormalized();
	std::vector<math::vector> simplex = { GJK_SupportFunction(aVertices, bVertices, direction) };
	direction = (math::vector::zero - simplex[0]).getNormalized();
	while (true)
	{
		math::vector supportPoint = GJK_SupportFunction(aVertices, bVertices, direction);
		if (supportPoint.dot(direction) < 0.f)
		{
			return Intersection{};
		}
		simplex.push_back(supportPoint);
		if (GJK_HandleSimplex(simplex, direction))
		{
			//return true;
			break;
		}
	}

	// EPA
	float minDistance = FLT_MAX;
	size_t minIdx = 0u;
	math::vector minNormal = math::vector::zero;

	while (math::IsNearlyEqual(minDistance, FLT_MAX))
	{
		for (size_t i = 0u; i < simplex.size(); ++i)
		{
			size_t j = (i + 1) % simplex.size();

			const math::vector edge = simplex[j] - simplex[i];
			math::vector edgeNormal = math::vector(edge.y, -edge.x, 0.f).getNormalized();
			float distance = edgeNormal.dot(simplex[i]);
			
			if (distance < 0)
			{
				edgeNormal *= -1.f;
				distance *= -1.f;
			}

			if (distance < minDistance)
			{
				minDistance = distance;
				minNormal = edgeNormal;
				minIdx = j;
			}
		}

		const math::vector supportPoint = GJK_SupportFunction(aVertices, bVertices, minNormal);
		const float supportDistance = minNormal.dot(supportPoint);

//		if (!math::IsNearlyZero(minDistance - supportDistance, math::k_default_epsilon))
		if (std::abs(minDistance - supportDistance) > math::k_default_epsilon)
		{
			minDistance = FLT_MAX;
			simplex.insert(simplex.begin() + minIdx, supportPoint);
		}
	}

	return Intersection{ true, minDistance, minNormal };
}

bool TestIntersection_SAT(const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices, math::vector& _outPenetrationAxis, float& _outPenetrationDepth)
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
		const size_t j = (i + 1) % bVertices.size();
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
	std::pair<math::vector, float> penetration = { math::vector::forward * FLT_MAX, FLT_MAX };
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
		else
		{
			const float maxStartOverlap = std::max(aProjectionMin, bProjectionMin);
			const float minEndOverlap = std::min(aProjectionMax, bProjectionMax);
			ASSERT(maxStartOverlap <= minEndOverlap, "Error in calculus.");

			const float penetrationCandidate = minEndOverlap - maxStartOverlap;
			if (penetrationCandidate < penetration.second)
			{
				penetration = {axis, penetrationCandidate };
			}
		}
	}

	_outPenetrationAxis = penetration.first;
	_outPenetrationDepth = penetration.second;
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
	// TESTING GJK
	{
		std::vector<math::vector> polygonA = { {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {2.f, 0.f, 0.f} };
		math::vector aCenter = { 0.25f, 0.25f, 0.f };
		//std::vector<math::vector> polygonA = { {-1.f, 0.f, 0.f}, {-1.f, 1.f, 0.f}, {0.f, 0.f, 0.f} };
		//math::vector aCenter = {-0.75f, 0.25f, 0.f};
		std::vector<math::vector> polygonB = { {1.f, 0.f, 0.f}, {3.f, 0.f, 0.f}, {3.f, 1.f, 0.f} };
		math::vector bCenter = { 2.75, 0.25, 0 };
		//const bool test = TestIntersection_GJK(polygonA, polygonB, aCenter, bCenter);
		int stop = 35;
	}
	static bool s_update = true;
	static constexpr float k_physicsTimeStep = 1.f / 144.f; // 1 simulation step per frame per 144 fps
	static float timer2 = 0.f;
	timer2 += deltaTime;

	float timeToSimulate = deltaTime;
	static math::vector force = math::vector{ 1.f, 1.f, 0.f };
	do
	{
		const float dt = timeToSimulate > k_physicsTimeStep ? k_physicsTimeStep : timeToSimulate;
		timeToSimulate -= dt;

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
				//const math::vector force = math::vector{ 1.f, 1.f, 0.f };// *1000000.f;
				math::vector accDelta = (force / body.m_shape.m_mass) * dt;
				body.m_acceleration += accDelta;
				body.m_velocity += body.m_acceleration * dt;
				body.m_position += body.m_velocity * dt;

				// Angular movement integration
				const math::vector torque = math::vector::zero;//math::vector::forward * 1000.f;
				body.m_angularAcceleration += (torque / body.m_shape.m_momentOfInertia) * dt;
				body.m_angularVelocity += body.m_angularAcceleration * dt;
				body.m_rotation += body.m_angularVelocity * dt;

				static float timer = 0.f;
				timer += dt;
				if (body.m_rotation.z >= 100.f)
				{
					int stop = 45;
				}

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
				math::vector{/*body.m_position.x*/ -body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ -body.m_shape.m_dimensions.y / 2.f, 0.f},
				math::vector{/*body.m_position.x*/ -body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ +body.m_shape.m_dimensions.y / 2.f, 0.f},
				math::vector{/*body.m_position.x*/ +body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ -body.m_shape.m_dimensions.y / 2.f, 0.f},
				math::vector{/*body.m_position.x*/ +body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ +body.m_shape.m_dimensions.y / 2.f, 0.f}
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
		std::vector<std::pair<
			std::pair<const PhysicsComponent&, std::vector<math::vector>>,
			std::pair<const PhysicsComponent&, std::vector<math::vector>>
			>> narrowPhasePairs;
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

				//s_update = !isIntersecting;
				if (isIntersecting)
				{
					// build polygons
					std::vector<math::vector> polygonA, polygonB;
					for (const Entity& entity : entities)
					{
						if (entity.m_entityId == body.m_entityId)
						{
							const MeshComponent& mesh = *(MeshComponent*)entity.m_components.at(ComponentType::MeshComponent);
							const TransformComponent& transform = *(TransformComponent*)entity.m_components.at(ComponentType::TransformComponent);

							const math::matrix scaleMatrix = math::ScaleMatrix(transform.m_scale.x, transform.m_scale.y, transform.m_scale.z);
							const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(transform.m_rotation.y), math::ConvertToRadians(transform.m_rotation.x), math::ConvertToRadians(transform.m_rotation.z));
							const math::matrix translationMatrix = math::TranslationMatrix(transform.m_position.x, transform.m_position.y, transform.m_position.z);
							const math::matrix modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;

							for (const auto vertex : mesh.m_vertices)
							{
								math::vector v = { vertex.first.m_x, vertex.first.m_y, vertex.first.m_z };
								polygonA.push_back(modelMatrix * v);
							}
						}

						if (entity.m_entityId == object->userData.m_entityId)
						{
							const MeshComponent& mesh = *(MeshComponent*)entity.m_components.at(ComponentType::MeshComponent);
							const TransformComponent& transform = *(TransformComponent*)entity.m_components.at(ComponentType::TransformComponent);

							const math::matrix scaleMatrix = math::ScaleMatrix(transform.m_scale.x, transform.m_scale.y, transform.m_scale.z);
							const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(transform.m_rotation.y), math::ConvertToRadians(transform.m_rotation.x), math::ConvertToRadians(transform.m_rotation.z));
							const math::matrix translationMatrix = math::TranslationMatrix(transform.m_position.x, transform.m_position.y, transform.m_position.z);
							const math::matrix modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;

							for (const auto vertex : mesh.m_vertices)
							{
								math::vector v = { vertex.first.m_x, vertex.first.m_y, vertex.first.m_z };
								polygonB.push_back(modelMatrix * v);
							}
						}
					}


					narrowPhasePairs.push_back({ { body, polygonA }, { object->userData, polygonB } });
				}
			}
		}

		static bool adjusted = false;

		// Narrow Phase
		for (const auto& pair : narrowPhasePairs)
		{
			math::vector penetrationAxis = math::vector::zero;
			float penetrationDepth = 0.f;
			const bool isIntersecting = TestIntersection_SAT(pair.first.second, pair.second.second, penetrationAxis, penetrationDepth);
			if (isIntersecting)
			{
				/*
				// project positions on penetrationAxis to calculate correct penetrationVector direction
				const float aProjection = pair.first.first.m_position.dot(penetrationAxis);
				const float bProjection = pair.second.first.m_position.dot(penetrationAxis);
				const float penetrationVectorSign = aProjection < bProjection ? -1.f : 1.f;
				
				force = penetrationAxis * penetrationDepth;
				if (pair.first.first.m_acceleration.length() > 0.f)
				{
					PhysicsComponent& aBody = const_cast<PhysicsComponent&>(pair.first.first);
					aBody.m_acceleration = math::vector::zero;
					aBody.m_velocity = math::vector::zero;
					aBody.m_position += penetrationAxis * penetrationDepth;
				}
				*/

				if (adjusted)
				{
					int stop = 34;
				}

				const Intersection intersection = TestIntersection_GJK(pair.first.second, pair.second.second, pair.first.first.m_position, pair.second.first.m_position);
				if (intersection.areIntersecting)
				{
					int stop = 34;

					force = -intersection.penetrationVector;// *pair.first.first.m_acceleration.length();// *intersection.penetrationDepth;
					if (pair.first.first.m_acceleration.length() > 0.f) // hack, should use something like IsDynamic intstead
					{
						PhysicsComponent& aBody = const_cast<PhysicsComponent&>(pair.first.first);
						aBody.m_acceleration = -intersection.penetrationVector * aBody.m_acceleration.length();
						aBody.m_velocity = math::vector::zero;
						aBody.m_position -= intersection.penetrationVector * intersection.penetrationDepth;
						adjusted = true;
					}
				}
			}
			//s_update = !isIntersecting;
		}

		// !!! TODO 
		// Calculate penetration depth
		int stop = 45;
	}
	while (!math::IsNearlyZero(timeToSimulate, math::k_default_epsilon));
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