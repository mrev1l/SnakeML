// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#pragma warning( disable : 4820 4061 4365 4668 26812 6319 6011)
// directx 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <directxmath.h>

// d3d12 extension library.
#include <d3dx12.h>
#pragma warning( default : 4820 4061 4365 4668 26812 6319 6011)

#if defined(max)
#undef max
#endif
#if defined(min)
#undef min
#endif