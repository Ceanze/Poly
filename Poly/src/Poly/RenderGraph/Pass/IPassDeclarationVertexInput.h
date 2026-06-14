#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class IPassDeclarationGraphicsPipeline;

	class IPassDeclarationVertexInput
	{
	public:
		virtual ~IPassDeclarationVertexInput() = default;

		virtual IPassDeclarationVertexInput& Binding(uint32 binding)                = 0;
		virtual IPassDeclarationVertexInput& Stride(uint32 stride)                  = 0;
		virtual IPassDeclarationVertexInput& VertexInputRate(EVertexInputRate rate) = 0;
		virtual IPassDeclarationVertexInput& Location(uint32 location)              = 0;
		virtual IPassDeclarationVertexInput& Format(EFormat format)                 = 0;
		virtual IPassDeclarationVertexInput& Offset(uint32 offset)                  = 0;

		/*
		 * Commits the current vertex input and begins building the next one.
		 */
		virtual IPassDeclarationVertexInput& AddVertexInput() = 0;

		/*
		 * Commits the current vertex input and returns to the pipeline declaration.
		 */
		virtual IPassDeclarationGraphicsPipeline& FinishVertexInput() = 0;
	};
} // namespace Poly
