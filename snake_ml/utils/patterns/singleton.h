// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

namespace snakeml
{
namespace patterns
{

template<class T>
class Singleton
{
public:
	static inline bool HasInstance() { return s_instance; }
	static inline T*   GetInstance() { return s_instance; }

protected:
	Singleton() { s_instance = static_cast<T*>(this); }
	virtual ~Singleton() { s_instance = nullptr; }

private:
	static T* s_instance;
};

template<class T>
T* Singleton<T>::s_instance = nullptr;

}
}