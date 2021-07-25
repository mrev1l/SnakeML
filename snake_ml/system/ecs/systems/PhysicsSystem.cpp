// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "PhysicsSystem.h"

#include "system/ecs/components/MeshComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include <utility>

// TODO : REPLACE WITH LOGGING
#include "system/drivers/win/os/WinDriver.h"
namespace snakeml
{
namespace system
{

void GJK_FindSupportPoint(const math::vector& supportDirection, const std::vector<math::vector>& vertices, std::pair<math::vector, float>& _outSupportPoint)
{
	for (const math::vector& v : vertices)
	{
		const float dot = supportDirection.dot(v);
		if (dot > _outSupportPoint.second)
		{
			_outSupportPoint = { v, dot };
		}
	}
}

math::vector GJK_SupportFunction(const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices, const math::vector& direction)
{
	std::pair<math::vector, float> furthestPointA = { math::vector::zero, -FLT_MAX }, furthestPointB = { math::vector::zero, -FLT_MAX };

	GJK_FindSupportPoint( direction, aVertices, furthestPointA);
	GJK_FindSupportPoint(-direction, bVertices, furthestPointB);

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
	const math::vector triProduct = triple_product(ab, ao, ab);
	
	math::vector abPerp = math::IsNearlyZero(triProduct.length()) ? perpendicular2d(ab) : triProduct;

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
	const math::vector abTriProduct = triple_product(ac, ab, ab);
	const math::vector acTriProduct = triple_product(ab, ac, ac);
	const math::vector abPerp = math::IsNearlyZero(abTriProduct.length()) ? perpendicular2d(ab) : abTriProduct;
	const math::vector acPerp = math::IsNearlyZero(acTriProduct.length()) ? perpendicular2d(ac) : acTriProduct;
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

		if (std::abs(minDistance - supportDistance) > math::k_default_epsilon)
		{
			minDistance = FLT_MAX;
			simplex.insert(simplex.begin() + minIdx, supportPoint);
		}
	}

	return Intersection{ true, minDistance, minNormal };
}

bool SAT_IsAxisPresent(const math::vector& axisCandidate, std::vector<math::vector>& axises)
{
	return std::find_if(axises.begin(), axises.end(), [axisCandidate](const math::vector& a)
		{
			float dot = axisCandidate.dot(a);
			return math::IsNearlyEqual(dot, 1.f, cosf(math::ConvertToRadians(1.f))) ||
				math::IsNearlyEqual(dot, -1.f, cosf(math::ConvertToRadians(1.f)));
		}) != axises.end();
}

void SAT_BuildAxises(const std::vector<math::vector>& vertices, std::vector<math::vector>& _outAxises)
{
	for (size_t i = 0u; i < vertices.size(); ++i)
	{
		const size_t j = (i + 1) % vertices.size();
		math::vector edge = vertices[j] - vertices[i];
		// calculate perpendicular vector in 2D
		edge = { -edge.y, edge.x, 0.f };
		const math::vector axisCandidate = edge.getNormalized();

		// don't allow the same axis to occur multiple times
		if (!SAT_IsAxisPresent(axisCandidate, _outAxises))
		{
			_outAxises.push_back(axisCandidate);
		}
	}
}

void SAT_ProjectPolygon(const math::vector& axis, const std::vector<math::vector>& vertices, float& _outProjectionMin, float& _outProjectionMax)
{
	for (const math::vector& vertex : vertices)
	{
		float projection = vertex.dot(axis);
		_outProjectionMin = std::min(_outProjectionMin, projection);
		_outProjectionMax = std::max(_outProjectionMax, projection);
	}
}

bool TestIntersection_SAT(const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices)
{
	std::vector<math::vector> axises;

	SAT_BuildAxises(aVertices, axises);
	SAT_BuildAxises(bVertices, axises);

	for (const math::vector& axis : axises)
	{
		float aProjectionMin = FLT_MAX, aProjectionMax = -FLT_MAX, bProjectionMin = FLT_MAX, bProjectionMax = -FLT_MAX;

		SAT_ProjectPolygon(axis, aVertices, aProjectionMin, aProjectionMax);
		SAT_ProjectPolygon(axis, bVertices, bProjectionMin, bProjectionMax);

		if (
			aProjectionMax <= bProjectionMin || // a is before b
			aProjectionMin >= bProjectionMax)	// a is after b
		{
			return false;
		}
	}

	return true; // no separating axis found
}

void PhysicsSystem::Update(double deltaTime)
{
	static constexpr float k_physicsTimeStep = 1.f / 144.f; // 1 simulation step per frame per 144 fps
	//deltaTime = k_physicsTimeStep; // debug
	float timeToSimulate = deltaTime;

	do
	{
		const float dt = timeToSimulate > k_physicsTimeStep ? k_physicsTimeStep : timeToSimulate;
		timeToSimulate -= dt;

		types::QuadTree<PhysicsComponent> qt(types::QuadTree<PhysicsComponent>::Rectangle{ {0.f, 0.f, 0.f}, {370.f, 370.f, 0.f} });
		std::vector<NarrowPhasePair> narrowPhase;

		PhysicsComponentIterator* bodies = (PhysicsComponentIterator*)ECSManager::GetInstance()->GetComponentsPool().GetComponents(ComponentType::PhysicsComponent);
		PhysicsComponent* bodiesArr = (PhysicsComponent*)bodies->GetData();

		SimulatePhysics(bodiesArr, bodies->Size(), dt);
		UpdateAABBs(bodiesArr, bodies->Size());
		PopulateQuadTree(qt, bodiesArr, bodies->Size());
		BroadPhaseStep(qt, bodiesArr, bodies->Size(), narrowPhase);
		NarrowPhaseIntersectionSolutionStep(narrowPhase);
	}
	while (!math::IsNearlyZero(timeToSimulate, math::k_default_epsilon));
}

void PhysicsSystem::SimulatePhysics(PhysicsComponent* physicsBodies, size_t bodiesCount, double dt)
{
	for (size_t idx = 0; idx < bodiesCount; ++idx)
	{
		PhysicsComponent& body = physicsBodies[idx];
		Entity* entityPtr = ECSManager::GetInstance()->GetEntity(body.m_entityId);
		ASSERT(entityPtr, "(PhysicsSystem::Update) : Invalid entity ptr.");

		TransformComponent& transform = *(TransformComponent*)entityPtr->m_components[ComponentType::TransformComponent];

		if (body.m_isDynamic)
		{
			SimulatePhysicsStep(body, transform, dt);
		}
	}
}

void PhysicsSystem::SimulatePhysicsStep(PhysicsComponent& body, TransformComponent& transform, double dt)
{
	// Linear movement integration
	body.m_velocity += body.m_acceleration * dt;
	body.m_position += body.m_velocity * dt;

	// Angular movement integration
	body.m_angularVelocity += body.m_angularAcceleration * dt;
	body.m_rotation += body.m_angularVelocity * dt;

	// Update transform
	transform.m_position = body.m_position;
	transform.m_rotation = body.m_rotation;
}

void PhysicsSystem::UpdateAABBs(PhysicsComponent* physicsBodies, size_t bodiesCount)
{
	for (size_t i = 0u; i < bodiesCount; ++i)
	{
		PhysicsComponent& body = physicsBodies[i];
		UpdateAABB(body);
	}
}

void PhysicsSystem::UpdateAABB(PhysicsComponent& body)
{
	// Create Bounding Box
	std::array<math::vector, 4> boundingBox =
	{
		math::vector{-body.m_shape.m_dimensions.x / 2.f, -body.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{-body.m_shape.m_dimensions.x / 2.f, +body.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{+body.m_shape.m_dimensions.x / 2.f, -body.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{+body.m_shape.m_dimensions.x / 2.f, +body.m_shape.m_dimensions.y / 2.f, 0.f}
	};

	// Calc transform matrix for Bounding Box vertices
	const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(body.m_rotation.y), math::ConvertToRadians(body.m_rotation.x), math::ConvertToRadians(body.m_rotation.z));
	const math::matrix translationMatrix = math::TranslationMatrix(body.m_position.x, body.m_position.y, body.m_position.z);
	const math::matrix transformMatrix = rotationMatrix * translationMatrix;

	// Transform Bounding Box vertices
	for (auto& vertex : boundingBox)
	{
		vertex = transformMatrix * vertex;
	}

	// Construct AABB
	body.m_aabb = { {FLT_MAX, FLT_MAX, 0.f}, {-FLT_MAX, -FLT_MAX, 0.f} };
	for (auto vertex : boundingBox)
	{
		body.m_aabb.min.x = std::min(body.m_aabb.min.x, vertex.x);
		body.m_aabb.min.y = std::min(body.m_aabb.min.y, vertex.y);
		body.m_aabb.max.x = std::max(body.m_aabb.max.x, vertex.x);
		body.m_aabb.max.y = std::max(body.m_aabb.max.y, vertex.y);
	}
}

void PhysicsSystem::PopulateQuadTree(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent* physicsBodies, size_t bodiesCount)
{
	for (size_t i = 0u; i < bodiesCount; ++i)
	{
		PhysicsComponent& body = physicsBodies[i];
		AddPhysicsBodyToQuadTree(quadTree, body);
	}
}

void PhysicsSystem::AddPhysicsBodyToQuadTree(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body)
{
	quadTree.AddObject(types::QuadTree<PhysicsComponent>::Object{ {body.m_position, {(body.m_aabb.max - body.m_aabb.min) / 2.f}}, body });
}

void PhysicsSystem::BroadPhaseStep(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent* physicsBodies, size_t bodiesCount, std::vector<NarrowPhasePair>& _outNarrowPhase)
{
	for (size_t i = 0u; i < bodiesCount; ++i)
	{
		PhysicsComponent& body = physicsBodies[i];
		if (!body.m_isDynamic)
		{
			continue;
		}

		CalculateBroadphaseForBody(quadTree, body, _outNarrowPhase);
	}
}

void PhysicsSystem::CalculateBroadphaseForBody(const types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body, std::vector<NarrowPhasePair>& _outNarrowPhase)
{
	const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
	const types::QuadTree<PhysicsComponent>::Rectangle boundary = { body.m_position, body.m_aabb.max - body.m_aabb.min };
	std::vector<const types::QuadTree<PhysicsComponent>::Object const*> objects;

	quadTree.GetObjects(boundary, objects);

	for (auto object : objects)
	{
		if (&object->userData == &body)
		{
			continue;
		}
		const types::AABB& a = body.m_aabb;
		const types::AABB& b = object->userData.m_aabb;

		const bool areIntersecting = types::TestIntersection_AABB_AABB(a, b);

		if (areIntersecting)
		{
			Polygon polygonA, polygonB;
			
			GeneratePolygon(body, polygonA);
			GeneratePolygon(object->userData, polygonB);
			
			NarrowPhaseBody narrowPhaseBodyA(body, std::forward<Polygon>(polygonA));
			NarrowPhaseBody narrowPhaseBodyB(object->userData, std::forward<Polygon>(polygonB));

			_outNarrowPhase.emplace_back(std::move(NarrowPhasePair(std::forward<NarrowPhaseBody>(narrowPhaseBodyA), std::forward<NarrowPhaseBody>(narrowPhaseBodyB))));
		}
	}
}

void PhysicsSystem::NarrowPhaseIntersectionSolutionStep(const std::vector<NarrowPhasePair>& narrowPhase)
{
	Intersection foundIntersection;
	for (const auto& pair : narrowPhase)
	{
		ResolveNarrowPhase(pair, foundIntersection);

		if (foundIntersection.areIntersecting)
		{
			ResolveIntersection(pair, foundIntersection);
		}
	}
}

void PhysicsSystem::ResolveNarrowPhase(const NarrowPhasePair& narrowPhase, Intersection& _outFoundGJK_Intersection)
{
	const bool isSAT_IntersectionFound = TestIntersection_SAT(narrowPhase.a.polygon, narrowPhase.b.polygon);
	if (isSAT_IntersectionFound)
	{
		_outFoundGJK_Intersection = TestIntersection_GJK(narrowPhase.a.polygon, narrowPhase.b.polygon, narrowPhase.a.physicsObject.m_position, narrowPhase.b.physicsObject.m_position);
	}
}

void PhysicsSystem::ResolveIntersection(const NarrowPhasePair& narrowPhase, const Intersection& intersection)
{
	if (narrowPhase.a.physicsObject.m_isDynamic)
	{
		narrowPhase.a.physicsObject.m_position -= intersection.penetrationVector * intersection.penetrationDepth;

		math::vector reflectedVelocity = narrowPhase.a.physicsObject.m_velocity - (-intersection.penetrationVector * 2.f * narrowPhase.a.physicsObject.m_velocity.dot(-intersection.penetrationVector));
		narrowPhase.a.physicsObject.m_velocity = reflectedVelocity;
		narrowPhase.a.physicsObject.m_acceleration = math::vector::zero;

		OutputDebugString(L"INTERSECTION\n");
	}
}

void PhysicsSystem::GeneratePolygon(const PhysicsComponent& body, Polygon& _outPolygon)
{
	const Entity* entityPtr = ECSManager::GetInstance()->GetEntity(body.m_entityId);
	ASSERT(entityPtr, "[PhysicsSystem::GeneratePolygon] : Missconfigured entity.");
	const Entity& entity = *entityPtr;

	const MeshComponent& mesh = *(MeshComponent*)entity.m_components.at(ComponentType::MeshComponent);
	const TransformComponent& transform = *(TransformComponent*)entity.m_components.at(ComponentType::TransformComponent);

	const math::matrix scaleMatrix = math::ScaleMatrix(transform.m_scale.x, transform.m_scale.y, transform.m_scale.z);
	const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(transform.m_rotation.y), math::ConvertToRadians(transform.m_rotation.x), math::ConvertToRadians(transform.m_rotation.z));
	const math::matrix translationMatrix = math::TranslationMatrix(transform.m_position.x, transform.m_position.y, transform.m_position.z);
	const math::matrix modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	for (const auto& vertex : mesh.m_vertices)
	{
		math::vector v = { vertex.first.m_x, vertex.first.m_y, vertex.first.m_z };
		_outPolygon.push_back(modelMatrix * v);
	}
}

}
}