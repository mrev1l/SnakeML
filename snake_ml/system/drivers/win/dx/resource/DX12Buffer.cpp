// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12Buffer.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

DX12Buffer::DX12Buffer(const std::wstring& name)
	: DX12Resource(name)
{
}

DX12Buffer::DX12Buffer( const D3D12_RESOURCE_DESC& resDesc,
	size_t numElements, size_t elementSize,
	const std::wstring& name )
	: DX12Resource(resDesc, nullptr, name)
{
	CreateViews(numElements, elementSize);
}

void DX12Buffer::CreateViews(size_t numElements, size_t elementSize)
{
	throw std::exception("Unimplemented function.");
}

}
#endif
}
