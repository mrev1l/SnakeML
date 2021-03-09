// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "RenderDriver.h"


namespace snakeml
{
namespace system
{

void IRenderDriver::Initialize()
{
	OnInitialize();
	m_isInitialized = true;
}

void IRenderDriver::Shutdown()
{
	OnShutdown();
	m_isInitialized = false;
}

bool IRenderDriver::IsInitialized() const
{
	return m_isInitialized;
}

void IRenderDriver::Render()
{
	OnRender();
	++m_frameCounter;
}

}
}
