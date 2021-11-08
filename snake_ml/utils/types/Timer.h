#pragma once
#include "utils/patterns/event.h"

namespace snakeml
{

class Timer
{
public:
	enum State
	{
		Dormant,
		Running
	};

	Timer() = default;
	~Timer() = default;

	void Start(float duration);
	void Update(float dt);
	void Reset();

	State GetState() const { return m_state; }

	Event<Timer, void> m_onElapsed;

private:
	State	m_state		= Dormant;
	float	m_duration	= 0.f;
	float	m_timer		= 0.f;
};

}