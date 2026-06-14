#pragma once

#include "IPassDeclarationVertexInput.h"
#include "Platform/API/GraphicsPipeline.h"

namespace Poly
{
	class PassDeclarationGraphicsPipeline;

	class PassDeclarationVertexInput : public IPassDeclarationVertexInput
	{
	public:
		explicit PassDeclarationVertexInput(PassDeclarationGraphicsPipeline& pipeline);

		PassDeclarationVertexInput& Binding(uint32 binding) override;
		PassDeclarationVertexInput& Stride(uint32 stride) override;
		PassDeclarationVertexInput& VertexInputRate(EVertexInputRate rate) override;
		PassDeclarationVertexInput& Location(uint32 location) override;
		PassDeclarationVertexInput& Format(EFormat format) override;
		PassDeclarationVertexInput& Offset(uint32 offset) override;

		PassDeclarationVertexInput&      AddVertexInput() override;
		PassDeclarationGraphicsPipeline& FinishVertexInput() override;

	private:
		PassDeclarationGraphicsPipeline& m_Pipeline;
		VertexInput                      m_Current = {};
	};
} // namespace Poly
