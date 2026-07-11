#pragma once

#include "IPassDeclarationColorBlendAttachment.h"
#include "Platform/API/GraphicsPipeline.h"

namespace Poly
{
	class PassDeclarationGraphicsPipeline;

	class PassDeclarationColorBlendAttachment : public IPassDeclarationColorBlendAttachment
	{
	public:
		explicit PassDeclarationColorBlendAttachment(PassDeclarationGraphicsPipeline& pipeline);

		IPassDeclarationColorBlendAttachment& BlendEnable(bool enable) override;
		IPassDeclarationColorBlendAttachment& SrcColorBlendFactor(EBlendFactor factor) override;
		IPassDeclarationColorBlendAttachment& DstColorBlendFactor(EBlendFactor factor) override;
		IPassDeclarationColorBlendAttachment& ColorBlendOp(EBlendOp op) override;
		IPassDeclarationColorBlendAttachment& SrcAlphaBlendFactor(EBlendFactor factor) override;
		IPassDeclarationColorBlendAttachment& DstAlphaBlendFactor(EBlendFactor factor) override;
		IPassDeclarationColorBlendAttachment& AlphaBlendOp(EBlendOp op) override;
		IPassDeclarationColorBlendAttachment& ColorWriteMask(FColorComponentFlag mask) override;

		IPassDeclarationColorBlendAttachment& AddColorBlendAttachment() override;
		IPassDeclarationGraphicsPipeline&     FinishColorBlendAttachment() override;

	private:
		PassDeclarationGraphicsPipeline& m_Pipeline;
		ColorBlendAttachmentDesc         m_Current = {};
	};
} // namespace Poly
