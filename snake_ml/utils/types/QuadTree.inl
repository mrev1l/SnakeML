#pragma once

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

	vector northwestOrigin = { m_boundary.origin.x - m_boundary.halfDimensions.x / 2.f, m_boundary.origin.y + m_boundary.halfDimensions.y / 2.f, 0.f };
	vector northeastOrigin = { m_boundary.origin.x + m_boundary.halfDimensions.x / 2.f, m_boundary.origin.y + m_boundary.halfDimensions.y / 2.f, 0.f };
	vector southwestOrigin = { m_boundary.origin.x - m_boundary.halfDimensions.x / 2.f, m_boundary.origin.y - m_boundary.halfDimensions.y / 2.f, 0.f };
	vector southeastOrigin = { m_boundary.origin.x + m_boundary.halfDimensions.x / 2.f, m_boundary.origin.y - m_boundary.halfDimensions.y / 2.f, 0.f };

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
	const auto IsObjPresent = [obj](const Object* a) -> bool
	{
		return &(a->userData) == &(obj.userData);
	};
	
	if (std::find_if(_outResult.begin(), _outResult.end(), IsObjPresent) != _outResult.end())
	{
		return;
	}
	
	_outResult.push_back(&obj);
}

template<typename T>
bool QuadTree<T>::Rectangle::Intersects(Rectangle other) const
{
	bool result = AABB::TestIntersection_AABB_AABB(
		{ origin - halfDimensions, origin + halfDimensions },
		{ other.origin - other.halfDimensions, other.origin + other.halfDimensions });

	return result;
}