// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "utils/patterns/singleton.h"
#include "utils/directX_utils.h"

#include <array>

namespace snakeml
{
namespace system
{

class IRenderDriver
	: public patterns::Singleton<IRenderDriver>
{
public:
	IRenderDriver() = default;
	virtual ~IRenderDriver() = default;

	void Initialize();
	void Shutdown();
	bool IsInitialized() const;

	void Render();

protected:
	virtual void OnInitialize() = 0;
	virtual void OnShutdown() = 0;
	virtual void OnRender() = 0;

private:
	bool m_isInitialized = false;
};

}
}