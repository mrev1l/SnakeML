#pragma once

namespace snakeml
{

	// TODO
#ifdef _WINDOWS
namespace win
{
	class WinDriver;
}
using OSImplementation = class win::WinDriver;
#endif

}
