#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class IPassDeclarationGraphicsPipeline;

	class IPassDeclarationColorBlendAttachment
	{
	public:
		virtual ~IPassDeclarationColorBlendAttachment() = default;

		virtual IPassDeclarationColorBlendAttachment& BlendEnable(bool enable)                 = 0;
		virtual IPassDeclarationColorBlendAttachment& SrcColorBlendFactor(EBlendFactor factor) = 0;
		virtual IPassDeclarationColorBlendAttachment& DstColorBlendFactor(EBlendFactor factor) = 0;
		virtual IPassDeclarationColorBlendAttachment& ColorBlendOp(EBlendOp op)                = 0;
		virtual IPassDeclarationColorBlendAttachment& SrcAlphaBlendFactor(EBlendFactor factor) = 0;
		virtual IPassDeclarationColorBlendAttachment& DstAlphaBlendFactor(EBlendFactor factor) = 0;
		virtual IPassDeclarationColorBlendAttachment& AlphaBlendOp(EBlendOp op)                = 0;
		virtual IPassDeclarationColorBlendAttachment& ColorWriteMask(FColorComponentFlag mask) = 0;

		/*
		 * Commits the current attachment and begins building the next one.
		 */
		virtual IPassDeclarationColorBlendAttachment& AddColorBlendAttachment() = 0;

		/*
		 * Commits the current attachment and returns to the pipeline declaration.
		 */
		virtual IPassDeclarationGraphicsPipeline& FinishColorBlendAttachment() = 0;
	};
} // namespace Poly
