#pragma once

template<class T>
inline T* Iterator::As()
{
	const std::type_info& tInfo = typeid(T);
	const std::type_info& thisTypeInfo = GetTypeInfo();

	if (tInfo == thisTypeInfo)
	{
		return static_cast<T*>(this);
	}

	return nullptr;
}

template<class T>
inline void IteratorBaseImpl<T>::Clear()
{
	T* ConcreteIterator = As<T>();
	IComponent::DeleteIterator(ConcreteIterator->At(0).GetComponentType(), this);
}

template<class T>
inline const std::type_info& IteratorBaseImpl<T>::GetTypeInfo() const
{
	return typeid(T);
}

template<class T>
inline T* IComponent::As()
{
	const std::type_info& tInfo = typeid(T);
	const std::type_info& thisTypeInfo = GetTypeInfo();

	if (tInfo == thisTypeInfo)
	{
		return static_cast<T*>(this);
	}

	return nullptr;
}

template<class T>
inline const std::type_info& ComponentBaseImpl<T>::GetTypeInfo() const
{
	return typeid(T);
}
