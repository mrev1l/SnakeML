// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#pragma warning( disable : 5039 5204 4820 4265 4365 4625 4626 4986)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h> // for commandlinetoargvw
#include <wrl.h>
#pragma warning( default : 5039 5204 4820 4265 4365 4625 4626 4986) //-V665

#if defined(CreateWindow)
#undef CreateWindow
#endif
#if defined(max)
#undef max
#endif
#if defined(min)
#undef min
#endif