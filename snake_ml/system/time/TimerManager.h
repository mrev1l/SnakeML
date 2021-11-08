#pragma once

#include "utils/patterns/singleton.h"
#include "utils/types/Timer.h"

/*
* Clearly an overkill for this project, but this was an interesting problem to consider.
*/

namespace snakeml
{

class TimerManager : public Singleton<TimerManager>
{
public:
	TimerManager();
	~TimerManager() = default;

	uint32_t StartTimer(float duration, std::function<void()> callback);
	void CancelTimer(uint32_t id);

	void Update(float dt);

private:
	static void OnTimerElapsed(uint32_t timerId, std::function<void()> clientCallback, TimerManager* me);

	std::list<Timer> m_timers;
	std::unordered_map<uint32_t, Timer*> m_activeTimers;
	std::unordered_map<uint32_t, Timer*> m_dormantTimers;
	
	std::vector<std::pair<uint32_t, Timer*>> m_timersToDeactivate;

	static inline uint32_t s_timerIdCounter = 0;
	static constexpr uint32_t k_defaultTimerCount = 10;
};

}