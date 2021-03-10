#pragma once
#include "system/drivers/RenderDriver.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

class DX12CommandList;

class DX12RenderCommand : public IRenderCommand
{
public:
	DX12RenderCommand() = default;
	virtual ~DX12RenderCommand() = default;

	virtual void Execute(std::shared_ptr<DX12CommandList> commandList) = 0;
};

}
#endif
}
}