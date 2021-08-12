// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "utils/patterns/singleton.h"

namespace snakeml
{

class InputManager
	: public Singleton<InputManager>
{
public:
	InputManager();
	~InputManager() = default;

private:
	void ProcessInput(uint64_t inputKey);
};

}
