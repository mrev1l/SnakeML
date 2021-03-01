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

class Entity;

class IRenderCommand
{
public:
	virtual ~IRenderCommand() = default;
};

class IRenderCommandFactory
{
public:
	virtual ~IRenderCommandFactory() = default;
	virtual void BuildRenderCommands(const Entity& entity, std::vector<std::unique_ptr<IRenderCommand>>& outRenderCommands) = 0;
};

class IRenderDriver
	: public patterns::Singleton<IRenderDriver>
{
public:
	IRenderDriver() = default;
	virtual ~IRenderDriver() = default;

	void Initialize();
	void Shutdown();
	bool IsInitialized() const;

	virtual void SubscribeForRendering(const Entity& renderable) = 0;

	void Render();

protected:
	virtual void OnInitialize() = 0;
	virtual void OnShutdown() = 0;
	virtual void OnRender() = 0;

	std::vector<std::unique_ptr<IRenderCommand>> m_renderCommands;
	std::unique_ptr<IRenderCommandFactory> m_renderCommandFactory;

private:
	bool m_isInitialized = false;
};

}
}