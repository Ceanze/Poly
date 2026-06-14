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

		PassDeclarationColorBlendAttachment& BlendEnable(bool enable) override;
		PassDeclarationColorBlendAttachment& SrcColorBlendFactor(EBlendFactor factor) override;
		PassDeclarationColorBlendAttachment& DstColorBlendFactor(EBlendFactor factor) override;
		PassDeclarationColorBlendAttachment& ColorBlendOp(EBlendOp op) override;
		PassDeclarationColorBlendAttachment& SrcAlphaBlendFactor(EBlendFactor factor) override;
		PassDeclarationColorBlendAttachment& DstAlphaBlendFactor(EBlendFactor factor) override;
		PassDeclarationColorBlendAttachment& AlphaBlendOp(EBlendOp op) override;
		PassDeclarationColorBlendAttachment& ColorWriteMask(FColorComponentFlag mask) override;

		PassDeclarationColorBlendAttachment& AddColorBlendAttachment() override;
		PassDeclarationGraphicsPipeline&     FinishColorBlendAttachment() override;

	private:
		PassDeclarationGraphicsPipeline& m_Pipeline;
		ColorBlendAttachmentDesc         m_Current = {};
	};
} // namespace Poly
