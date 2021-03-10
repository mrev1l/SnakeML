// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "system/Application.h"

#include "lib_includes/directX_includes.h"
#include <dxgidebug.h>
void ReportLiveObjects()
{
	Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
	{
		dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	}
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	using namespace snakeml;

	system::Application snake_ml;
	snake_ml.Initialize();
	snake_ml.Run();
	snake_ml.Shutdown();

	atexit(&ReportLiveObjects);

	return 0;
}