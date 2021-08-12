#pragma once

namespace snakeml
{

template<typename T>
class QuadTree
{
public:
	struct Rectangle
	{
		vector origin;
		vector halfDimensions;

		bool Intersects(Rectangle other) const;
	};

	struct Object
	{
		Rectangle shape;
		T& userData;
	};

	QuadTree(Rectangle boundary);
	~QuadTree() = default;

	bool AddObject(const Object& object);
	void GetObjects(Rectangle boundary, std::vector<const Object const*>& _outResult) const;

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

#include "QuadTree.inl"

}
