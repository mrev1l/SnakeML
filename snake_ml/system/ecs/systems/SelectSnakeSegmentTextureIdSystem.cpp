// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "SelectSnakeSegmentTextureIdSystem.h"

#include "system/ecs/components/EntityControllerComponent.h"
#include "system/ecs/components/ParentComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/components/MaterialComponent.h"

#include "system/ecs/ECSManager.h"

namespace snakeml
{

// Only for pairs of std::hash-able types for simplicity.
// You can of course template this struct to allow other hash functions
struct pair_hash {
	template <class T1, class T2>
	std::size_t operator () (const std::pair<T1, T2>& p) const {
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);

		// Mainly for demonstration purposes, i.e. works but is overly simple
		// In the real world, use sth. like boost.hash_combine
		return h1 ^ h2;
	}
};

//void SelectSnakeSegmentTextureIdSystem::Update(float dt)
void SelectSnakeSegmentTextureIdSystem::Execute()
{
	struct cache
	{
		float headYaw;
		float secondYaw;
		MaterialComponent::TextureId secondId;
	};
	static std::vector<cache> s_calcCache;


	std::vector<Entity*> snakeSegments; // TODO try std reference_wrapper

	EntityControllerComponentIterator* snakeHeadContainer = ECSManager::GetInstance()->GetComponents<EntityControllerComponentIterator>();
	ASSERT(snakeHeadContainer->Size() == 1, "[SelectSnakeSegmentTextureIdSystem::Update] : Unexpected number of snake heads.");

	snakeSegments.push_back(&ECSManager::GetInstance()->GetEntity(snakeHeadContainer->At(0).m_entityId));

	uint32_t childId = -1;
	ASSERT(snakeSegments.back()->m_components.contains(ComponentType::ParentComponent), "[SelectSnakeSegmentTextureIdSystem::Update] : Misconfigured parent entity.");
	childId = snakeSegments.back()->m_components.at(ComponentType::ParentComponent)->As<ParentComponent>()->m_childId;

	while (childId != -1)
	{
		snakeSegments.push_back(&ECSManager::GetInstance()->GetEntity(childId));

		ASSERT(snakeSegments.back()->m_components.contains(ComponentType::ParentComponent), "[SelectSnakeSegmentTextureIdSystem::Update] : Misconfigured parent entity.");
		childId = snakeSegments.back()->m_components.at(ComponentType::ParentComponent)->As<ParentComponent>()->m_childId;
	}
	
	if (snakeSegments.size() == 1)
	{
		return;
	}

	// 0 segment
	snakeSegments.front()->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::Head0;

	if (snakeSegments.front()->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z != 0.f)
	{
		int stop = 45;
	}
	enum SegmentDirection
	{
		Left, Right, Up, Down, Invalid
	};

	// New take 2

	auto SelectTextureId = [](Entity* segment, SegmentDirection headDirection, SegmentDirection headOldDirection) -> void
	{
		if (headDirection == Right && headOldDirection == Right)
		{
			segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightVertical;
			segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = -90.f;
			segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = -90.f;
		}
		else if (headDirection == Left && headOldDirection == Left)
		{
			segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightVertical;
			segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 90.f;
			segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 90.f;
		}
		else if (headDirection == Up && headOldDirection == Up)
		{
			segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightVertical;
			segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 0.f;
			segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 0.f;
		}
		else if (headDirection == Down && headOldDirection == Down)
		{
			segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightVertical;
			segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 180.f;
			segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 180.f;
		}
		else
		{
			if (headDirection == Up && headOldDirection == Right)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::LeftUp;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 0.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 0.f;
			}
			else if (headDirection == Right && headOldDirection == Down)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::LeftUp;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = -90.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = -90.f;
			}
			else if (headDirection == Down && headOldDirection == Left)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::LeftUp;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 180.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 180.f;
			}
			else if (headDirection == Left && headOldDirection == Up)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::LeftUp;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 90.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 90.f;
			}
			else if (headDirection == Up && headOldDirection == Left)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::DownRight;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 0.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 0.f;
			}
			else if (headDirection == Left && headOldDirection == Down)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::DownRight;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 90.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 90.f;
			}
			else if (headDirection == Down && headOldDirection == Right)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::DownRight;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 180.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 180.f;
			}
			else if (headDirection == Right && headOldDirection == Up)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::DownRight;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = -90.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = -90.f;
			}
		}
	};

	for (uint32_t i = snakeSegments.size() - 1; i > 0; --i)
	{
		Entity* parent = snakeSegments[i - 1];
		Entity* current = snakeSegments[i];

		const vector headRotation = parent->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation;
		const SegmentDirection headDirection =
			IsNearlyEqual(headRotation.z, 0.f) ? Up :
			IsNearlyEqual(headRotation.z, 180.f) ? Down :
			IsNearlyEqual(headRotation.z, 90.f) ? Left :
			IsNearlyEqual(headRotation.z, -90.f) ? Right :
			Invalid;

		if (headDirection == Invalid)
		{
			int stop = 34;
		}

		const vector headOldRotation = parent->m_components.at(ComponentType::ParentComponent)->As<ParentComponent>()->m_parentPreviousMovementTickRotation;
		const SegmentDirection headOldDirection =
			IsNearlyEqual(headOldRotation.z, 0.f) ? Up :
			IsNearlyEqual(headOldRotation.z, 180.f) ? Down :
			IsNearlyEqual(headOldRotation.z, 90.f) ? Left :
			IsNearlyEqual(headOldRotation.z, -90.f) ? Right :
			Invalid;

		if (headOldDirection == Invalid)
		{
			int stop = 34;
		}

		SelectTextureId(current, headDirection, headOldDirection);
		if (i == 2)
		{
			//s_calcCache.push_back({
			//	headRotation.z,
			//	current->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z,
			//	current->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId });
			//if (false)
			//{
			//	s_calcCache.clear();
			//}
		}
	}
	//ECSManager::GetInstance()->UnscheduleSystem(this);
	return;
	// New take 1
	// 1 segment
	// FIX TransformComponent)->As<TransformComponent
	bool shouldUpdateIds = false;
	MaterialComponent::TextureId new1stSegmentId = snakeSegments[1]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId;

	static const std::unordered_map<std::pair<SegmentDirection, SegmentDirection>, MaterialComponent::TextureId, pair_hash> textureIdMap =
	{
		{ { Right, Up }, MaterialComponent::TextureId::UpRight},
		{ { Up, Right }, MaterialComponent::TextureId::LeftUp},

		{ { Down, Left }, MaterialComponent::TextureId::UpRight},
		{ { Left, Down }, MaterialComponent::TextureId::LeftUp},

		{ { Up, Left }, MaterialComponent::TextureId::DownRight},
		{ { Left, Up }, MaterialComponent::TextureId::LeftDown},

		{ { Right, Down }, MaterialComponent::TextureId::DownRight},
		{ { Down, Right }, MaterialComponent::TextureId::LeftDown},

		{ { Up, Up }, MaterialComponent::TextureId::StraightVertical},
		{ { Down, Down }, MaterialComponent::TextureId::StraightVertical},

		{ { Left, Left }, MaterialComponent::TextureId::StraightHorizontal},
		{ { Right, Right }, MaterialComponent::TextureId::StraightHorizontal},
	};

	{
		const vector headRotation = snakeSegments.front()->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation;
		const SegmentDirection headDirection =
			IsNearlyEqual(headRotation.z, 0.f) ? Up :
			IsNearlyEqual(headRotation.z, 180.f) ? Down :
			IsNearlyEqual(headRotation.z, 90.f) ? Left :
			IsNearlyEqual(headRotation.z, -90.f) ? Right :
			Invalid;

		if (headDirection == Invalid)
		{
			int stop = 34;
		}

		const vector headOldRotation = snakeSegments.front()->m_components.at(ComponentType::ParentComponent)->As<ParentComponent>()->m_parentPreviousMovementTickRotation;
		const SegmentDirection headOldDirection =
			IsNearlyEqual(headOldRotation.z, 0.f) ? Up :
			IsNearlyEqual(headOldRotation.z, 180.f) ? Down :
			IsNearlyEqual(headOldRotation.z, 90.f) ? Left :
			IsNearlyEqual(headOldRotation.z, -90.f) ? Right :
			Invalid;

		if (headOldDirection == Invalid)
		{
			int stop = 34;
		}

		if (textureIdMap.contains({ headDirection, headOldDirection }))
		{
			const MaterialComponent::TextureId newId = textureIdMap.at({ headDirection, headOldDirection });;
			shouldUpdateIds = snakeSegments[1]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId != newId;
			new1stSegmentId = textureIdMap.at({ headDirection, headOldDirection });
			snakeSegments[1]->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 0.f;
		}
		else
		{
			int stop = 9;
		}
	}
	// [n -> 1) segments
	if (!shouldUpdateIds)
	{
		//if (snakeSegments.size() <= 2)
		//{
		//	return;
		//}

		//// n segment update
		//if (IsNearlyZero((snakeSegments.back()->m_components.at(ComponentType::ParentComponent)->As<ParentComponent>()->m_parentPreviousMovementTickRotation.z -
		//	snakeSegments.back()->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z)))
		//{
		//	if (IsNearlyEqual(abs(snakeSegments.back()->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z), 90.f))
		//	{
		//		snakeSegments.back()->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightHorizontal;
		//	}
		//	else if (
		//		IsNearlyEqual(snakeSegments.back()->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z, 0.f) ||
		//		IsNearlyEqual(snakeSegments.back()->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z, 180.f)
		//		)
		//	{
		//		snakeSegments.back()->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightVertical;
		//	}
		//	else
		//	{
		//		int stop = 4;
		//	}
		//}
		return;
	}

	for (uint32_t i = snakeSegments.size() - 1; i > 1; --i)
	{
		snakeSegments[i]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId =
			snakeSegments[i - 1]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId;
		
		snakeSegments[i]->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z =
			snakeSegments[i - 1]->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z;
	}

	snakeSegments[1]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = new1stSegmentId;

	

	return;
	/// OLD
	//enum SegmentDirection
	//{
	//	Left, Right, Up, Down, Invalid
	//};

	//SegmentDirection parentDirection = Invalid, childDirection = Invalid;
	//MaterialComponent::TextureId tailParentTexture = MaterialComponent::TextureId::Count;
	//{
	//	tailParentTexture = (*(snakeSegments.rbegin() + 1))->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId;
	//}

	//static const std::unordered_map<std::pair<SegmentDirection, SegmentDirection>, MaterialComponent::TextureId, pair_hash> textureIdMap =
	//{
	//	{ { Right, Up }, MaterialComponent::TextureId::DownRight},
	//	{ { Up, Right }, MaterialComponent::TextureId::DownRight},

	//	{ { Down, Left }, MaterialComponent::TextureId::LeftDown},
	//	{ { Left, Down }, MaterialComponent::TextureId::LeftDown},

	//	{ { Up, Left }, MaterialComponent::TextureId::LeftUp},
	//	{ { Left, Up }, MaterialComponent::TextureId::LeftUp},

	//	{ { Right, Down }, MaterialComponent::TextureId::UpRight},
	//	{ { Down, Right }, MaterialComponent::TextureId::UpRight}
	//};

	//for (uint32_t i = 1; i < snakeSegments.size() - 1; ++i)
	//{
	//	const vector parentPos = snakeSegments[i - 1]->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_position;
	//	const vector childPos = snakeSegments[i + 1]->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_position;
	//	const vector segmentPos = snakeSegments[i]->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_position;

	//	const vector segmentToParent = parentPos - segmentPos;
	//	const vector segmentToChild = childPos - segmentPos;

	//	parentDirection = IsNearlyZero(segmentToParent.x) ? (segmentToParent.y > 0.f ? Up : Down) : (segmentToParent.x > 0.f ? Right : Left);
	//	childDirection = IsNearlyZero(segmentToChild.x) ? (segmentToChild.y > 0.f ? Up : Down) : (segmentToChild.x > 0.f ? Right : Left);

	//	if (IsNearlyZero(segmentToParent.x) && IsNearlyZero(segmentToChild.x))
	//	{
	//		snakeSegments[i]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightVertical;
	//		snakeSegments[i]->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = segmentToParent.y > 0.f ? 0.f : 180.f;
	//	}
	//	else if (IsNearlyZero(segmentToParent.y) && IsNearlyZero(segmentToChild.y))
	//	{
	//		snakeSegments[i]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightHorizontal;
	//		snakeSegments[i]->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = segmentToParent.x > 0.f ? 0.f : 180.f;
	//	}
	//	else
	//	{
	//		snakeSegments[i]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = textureIdMap.at({parentDirection, childDirection});
	//	}
	//	/*else if (
	//		((IsNearlyZero(segmentToParent.y) && segmentToParent.x > 0) && (IsNearlyZero(segmentToChild.x) && segmentToChild.y > 0)) ||
	//		((IsNearlyZero(segmentToParent.x) && segmentToParent.y > 0) && (IsNearlyZero(segmentToChild.y) && segmentToChild.x > 0)))
	//	{
	//		snakeSegments[i]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::DownRight;
	//	}
	//	else if (
	//		((IsNearlyZero(segmentToParent.x) && segmentToParent.y < 0) && (IsNearlyZero(segmentToChild.y) && segmentToChild.x < 0)) ||
	//		((IsNearlyZero(segmentToParent.y) && segmentToParent.x < 0) && (IsNearlyZero(segmentToChild.x) && segmentToChild.y < 0)))
	//	{
	//		snakeSegments[i]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::LeftDown;
	//	}
	//	else if (
	//		((IsNearlyZero(segmentToParent.x) && segmentToParent.y > 0) && (IsNearlyZero(segmentToChild.y) && segmentToChild.x < 0)) ||
	//		((IsNearlyZero(segmentToParent.y) && segmentToParent.x < 0) && (IsNearlyZero(segmentToChild.x) && segmentToChild.y > 0)))
	//	{
	//		snakeSegments[i]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::LeftUp;
	//	}
	//	else if (
	//		((IsNearlyZero(segmentToParent.y) && segmentToParent.x > 0) && (IsNearlyZero(segmentToChild.x) && segmentToChild.y < 0)) ||
	//		((IsNearlyZero(segmentToParent.x) && segmentToParent.y < 0) && (IsNearlyZero(segmentToChild.y) && segmentToChild.x > 0)))
	//	{
	//		snakeSegments[i]->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::UpRight;
	//	}*/
	//}

	//// Tail
	//const Entity* tailParent = *(snakeSegments.rbegin() + 1);
	//Entity* tail = snakeSegments.back();
	//const vector segmentToParent = tailParent->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_position -
	//	tail->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_position;

	//ASSERT(segmentToParent.length() > 0.f, "[SelectSnakeSegmentTextureIdSystem::Update] : Snake tail is stuck in the prev segment.");
	//SegmentDirection toDirection = Invalid;
	//SegmentDirection fromDirection = Invalid;



	//if (IsNearlyZero(segmentToParent.x))
	//{
	//	toDirection = tailParent->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z == 0.f ?
	//		SegmentDirection::Up : SegmentDirection::Down;
	//	fromDirection = tail->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z == 0.f ?
	//		SegmentDirection::Left : SegmentDirection::Right;
	//}

	//if (textureIdMap.contains({ fromDirection, toDirection }))
	//{
	//	tail->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = textureIdMap.at({ fromDirection, toDirection });
	//}
	//else if (IsNearlyZero(segmentToParent.x))
	//{
	//	tail->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::TailVertical;
	//	tail->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = segmentToParent.y > 0.f ? 0.f : 180.f;
	//}
	//else if (IsNearlyZero(segmentToParent.y))
	//{
	//	tail->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::TailHorizontal;
	//	tail->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = segmentToParent.x > 0.f ? 0.f : 180.f;
	//}

	/*if (tail->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId == MaterialComponent::TextureId::TailHorizontal)
	{
		if (IsNearlyZero(segmentToParent.x))
		{
			SegmentDirection toDirection = tailParent->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z == 0.f ?
				SegmentDirection::Up : SegmentDirection::Down;
			SegmentDirection fromDirection = tail->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z == 0.f ?
				SegmentDirection::Left : SegmentDirection::Right;

			tail->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = textureIdMap.at({ fromDirection, toDirection });
		}
	}
	else if (IsNearlyZero(segmentToParent.x))
	{
		tail->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::TailVertical;
		tail->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = segmentToParent.y > 0.f ? 0.f : 180.f;
	}
	else if (IsNearlyZero(segmentToParent.y))
	{
		tail->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::TailHorizontal;
		tail->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = segmentToParent.x > 0.f ? 0.f : 180.f;
	}*/
	
	
	
	

	/////  ALSO CALCULATE ROTATION ?!
	//snakeSegments.back()->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::TailVertical;
	//const vector tailToParent = 
	//	(*(snakeSegments.rbegin() + 1))->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_position -
	//	(*snakeSegments.rbegin())->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_position;
	//float yaw = 0;
	//if (tailToParent.y < 0)
	//{
	//	yaw = 180.f;
	//}
	//else if (tailToParent.x > 0)
	//{
	//	yaw = -90.f;
	//}
	//else if (tailToParent.x < 0)
	//{
	//	yaw = 90.f;
	//}
	//(*snakeSegments.rbegin())->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = yaw;
}

}