#pragma once
#include "DX12RenderCommand.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

class DX12RenderCommandFactory : public IRenderCommandFactory
{
public:
	~DX12RenderCommandFactory() = default;
	virtual void BuildRenderCommands(const Entity& entity, std::vector<std::unique_ptr<IRenderCommand>>& _outRenderCommands) override;

private:
	matrix BuildRenderCommands_CalculateCameraMatrix();
	void BuildRenderCommands_CalculateEntityMatrices(const Entity& entity, matrix& _outScaleMatrix, matrix& _outRotationMatrix, matrix& _outTranslationMatrix);

	void BuildRenderCommands_Main(const Entity& entity, const matrix& camera, const matrix& ortho, const matrix& scale, const matrix& rotation,
		const matrix& translation, std::vector<std::unique_ptr<IRenderCommand>>& outRenderCommands);
	void BuildRenderCommands_Debug(const Entity& entity, const matrix& camera, const matrix& ortho, const matrix& translation, std::vector<std::unique_ptr<IRenderCommand>>& outRenderCommands);
};

}
#endif
}
