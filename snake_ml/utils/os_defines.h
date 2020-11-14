#pragma once

namespace snakeml
{
namespace system
{

#ifdef _WINDOWS
		namespace win
		{
			class WinDriver;
		}
		using OSImplementation = class win::WinDriver;
#endif

}
}