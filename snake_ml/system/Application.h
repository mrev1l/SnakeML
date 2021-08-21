// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "input/InputKeys.h"
#include "utils/patterns/singleton.h"

namespace snakeml
{

class Application
	: public Singleton<Application>
{
public:
	Application();
	~Application();

	void Initialize();
	void Run();
	void Shutdown();

private:
	void OnInput(InputKey inputKey);
	void Update(float dt);
	void Render();
};

}
