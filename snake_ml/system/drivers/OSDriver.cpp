// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "OSDriver.h"

namespace snakeml
{

void IOSDriver::SendInputEvent(uint64_t keyCode)
{
	m_onInputEvent.Dispatch(keyCode);
}

void IOSDriver::SendUpdateEvent(float dt)
{
	m_onUpdateEvent.Dispatch(dt);
}

}
