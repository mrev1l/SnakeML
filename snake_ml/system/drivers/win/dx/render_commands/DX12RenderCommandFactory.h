#pragma once
#include "DX12RenderCommand.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

class DX12RenderCommandFactory : public IRenderCommandFactory
{
public:
	~DX12RenderCommandFactory() = default;
	virtual void BuildRenderCommands(const Entity& entity, std::vector<std::unique_ptr<IRenderCommand>>& outRenderCommands) override;
};

}
#endif
}
}