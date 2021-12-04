#pragma once

#include "system/ecs/ISystem.h"

#include "system/ecs/components/MaterialComponent.h"

namespace snakeml
{

class SelectSnakeSegmentTextureIdSystem : public ISystemCastableImpl<SelectSnakeSegmentTextureIdSystem>
{
public:
	SelectSnakeSegmentTextureIdSystem(std::vector<uint32_t> targetEntities = std::vector<uint32_t>()) : ISystemCastableImpl<SelectSnakeSegmentTextureIdSystem>(targetEntities) {};
	virtual ~SelectSnakeSegmentTextureIdSystem() = default;

	void Update(float dt) override;

private:
	enum SegmentDirection
	{
		Left, Right, Up, Down, Invalid
	};

	void UpdateHasMovedToggle(Entity* snakeHead);
	static std::vector<Entity*> BuildSnakeSegmentsVector();

	vector	m_oldHeadPosition	= vector::zero;
	bool	m_hasHeadMoved		= false;

	static SegmentDirection CalculateSegmentDirection(float yaw);

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

	using ParentSegmentTranslation	= std::pair<SegmentDirection, SegmentDirection>;
	using SegmentTextureId			= std::pair<MaterialComponent::TextureId, float>;
	static const std::unordered_map<ParentSegmentTranslation, SegmentTextureId, pair_hash> k_parentSegmentMovementToTextureIdMap;
};

}