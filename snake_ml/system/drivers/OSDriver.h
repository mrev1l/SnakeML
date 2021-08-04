// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "utils/patterns/singleton.h"
#include "utils/patterns/event.h"
#include "utils/os_defines.h"

#include <functional>

namespace snakeml
{
namespace system
{

class IOSDriver : public patterns::Singleton<IOSDriver>
{
public:
	IOSDriver() = default;
	virtual ~IOSDriver() = default;

	void Initialize()		{ OnInitialize(); }
	void RunOSMainLoop()	{ OnRunOSMainLoop(); }

	void Quit()				{ OnQuit(); }

	void Update()			{ OnUpdate(); }

	virtual void GetAppDimensions(uint32_t& _outWidth, uint32_t& _outHeight) = 0;

	patterns::Event<OSImplementation, uint64_t>	m_onInputEvent;
	patterns::Event<OSImplementation, double>	m_onUpdateEvent;

protected:
	virtual void OnInitialize() = 0;
	virtual void OnRunOSMainLoop() = 0;
	virtual void OnQuit() = 0;

	virtual void OnUpdate() = 0;
};

}
}