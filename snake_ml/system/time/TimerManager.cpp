// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "TimerManager.h"

namespace snakeml
{

TimerManager::TimerManager()
{
	m_timers.resize(k_defaultTimerCount);

	for (Timer& timer : m_timers)
	{
		m_dormantTimers[s_timerIdCounter++] = &timer;
	}
}

uint32_t TimerManager::StartTimer(float duration, std::function<void()> callback)
{
	if (m_dormantTimers.empty())
	{
		m_timers.push_back(Timer());
		m_dormantTimers[s_timerIdCounter++] = &m_timers.back();
	}

	std::unordered_map<uint32_t, Timer*>::iterator timerToStartIt = m_dormantTimers.begin();

	m_activeTimers.insert(*timerToStartIt);

	const uint32_t timerId = timerToStartIt->first;
	
	timerToStartIt->second->m_onElapsed.Subscribe(this, std::bind(&TimerManager::OnTimerElapsed, timerId, callback, this));
	timerToStartIt->second->Start(duration);

	m_dormantTimers.erase(timerId);

	return timerId;
}

void TimerManager::CancelTimer(uint32_t id)
{
	ASSERT(m_activeTimers.contains(id), "[TimerManager::CancelTimer] : Trying to cancel timer that is not active.");

	m_activeTimers.at(id)->Reset();
}

void TimerManager::Update(float dt)
{
	for (auto activeTimer : m_activeTimers)
	{
		activeTimer.second->Update(dt);
	}

	for (auto timer : m_timersToDeactivate)
	{
		m_dormantTimers[timer.first] = m_activeTimers[timer.first];
		m_activeTimers.erase(timer.first);
	}
	m_timersToDeactivate.clear();
}

void TimerManager::OnTimerElapsed(uint32_t timerId, std::function<void()> clientCallback, TimerManager* me)
{
	me->m_timersToDeactivate.push_back({ timerId, me->m_activeTimers[timerId] });
	clientCallback();
}



}