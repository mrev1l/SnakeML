// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "utils/patterns/singleton.h"
#include "utils/patterns/event.h"
#include "utils/os_defines.h"

namespace snakeml
{

class IOSDriver : public Singleton<IOSDriver>
{
public:
	IOSDriver() = default;
	virtual ~IOSDriver() = default;

	void Initialize()		{ OnInitialize(); }
	void RunOSMainLoop()	{ OnRunOSMainLoop(); }

	void Quit()				{ OnQuit(); }

	void Update()			{ OnUpdate(); }

	virtual void GetAppDimensions(uint32_t& _outWidth, uint32_t& _outHeight) const = 0;

	virtual void LogMessage(std::wstring msg) const = 0;

	Event<IOSDriver, uint64_t>	m_onKeyDownEvent;
	Event<IOSDriver, uint64_t>	m_onKeyUpEvent;

	Event<IOSDriver, float>		m_onUpdateEvent;

protected:
	void SendKeyDownEvent(uint64_t);
	void SendKeyUpEvent(uint64_t);
	void SendUpdateEvent(float);

	virtual void OnInitialize() = 0;
	virtual void OnRunOSMainLoop() = 0;
	virtual void OnQuit() = 0;

	virtual void OnUpdate() = 0;
};

}
