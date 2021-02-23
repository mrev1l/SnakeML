// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "utils/patterns/singleton.h"
#include "utils/win_utils.h"

namespace snakeml
{

class Application
	: public patterns::Singleton<Application>
{
public:
	Application();
	~Application();

	void Initialize();
	void Run();
	void Shutdown();

	static uint64_t s_frameCounter;

private:
	void Update();
	void Render();
	void Resize(uint32_t width, uint32_t height);
};

}