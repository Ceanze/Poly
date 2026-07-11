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

		IPassDeclarationVertexInput& Binding(uint32 binding) override;
		IPassDeclarationVertexInput& Stride(uint32 stride) override;
		IPassDeclarationVertexInput& VertexInputRate(EVertexInputRate rate) override;
		IPassDeclarationVertexInput& Location(uint32 location) override;
		IPassDeclarationVertexInput& Format(EFormat format) override;
		IPassDeclarationVertexInput& Offset(uint32 offset) override;

		IPassDeclarationVertexInput&      AddVertexInput() override;
		IPassDeclarationGraphicsPipeline& FinishVertexInput() override;

	private:
		PassDeclarationGraphicsPipeline& m_Pipeline;
		VertexInput                      m_Current = {};
	};
} // namespace Poly
