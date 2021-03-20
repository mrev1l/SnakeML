// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "utils/patterns/singleton.h"

namespace snakeml
{
namespace system
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

private:
	void Update(double dt);
	void Render();
	void Resize(uint32_t width, uint32_t height);
};

}
}