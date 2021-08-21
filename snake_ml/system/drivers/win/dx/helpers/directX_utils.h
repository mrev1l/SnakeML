// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "lib_includes/directX_includes.h"

#pragma warning(push)
#pragma warning( disable : 4820)
#include <chrono>
#pragma warning(pop)
#include <dxgidebug.h>

namespace snakeml
{

#ifdef _WINDOWS
namespace win
{

class DX12Utils
{
public:
	enum class DX12ShaderSemanticName : uint32_t
	{
		Position	= 0,
		TexCoord	= 1,
		Color		= 2
	};

	static void EnableDebugLayer();
	static Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);
	static Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
	static bool CheckTearingSupport();
	static Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
		uint32_t width, uint32_t height, uint32_t bufferCount);

	static const char* GetShaderSemanticNameStr(DX12ShaderSemanticName nameId) { return s_semanticNameMap.at(nameId); }

private:
	static void ReportLiveObjects();

	static std::map<DX12ShaderSemanticName, const char*> s_semanticNameMap;
};


}
#endif

}