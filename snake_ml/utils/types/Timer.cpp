// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "Timer.h"

namespace snakeml
{

void Timer::Start(float duration)
{
	ASSERT(!IsNearlyZero(duration), "[Timer::Timer] : Setting up a timer with 0 duration.");
	m_duration = duration;
	m_state = Running;
}

void Timer::Update(float dt)
{
	if (m_state == Dormant)
	{
		ASSERT(false, "[Timer::Update] : Updating a dormant timer");
		return;
	}

	m_timer += dt;
	if (m_timer >= m_duration)
	{
		m_onElapsed.Dispatch();
		Reset();
	}
}

void Timer::Reset()
{
	m_timer = 0.f;
	m_duration = 0.f;
	m_state = Dormant;

	m_onElapsed.Reset();
}

}