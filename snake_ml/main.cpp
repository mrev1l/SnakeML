// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "system/Application.h"

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	using namespace snakeml;

	Application snake_ml;
	snake_ml.Initialize();
	snake_ml.Run();
	snake_ml.Shutdown();

	return 0;
}