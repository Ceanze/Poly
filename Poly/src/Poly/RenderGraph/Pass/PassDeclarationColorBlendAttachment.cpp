#include "PassDeclarationColorBlendAttachment.h"

#include "PassDeclarationGraphicsPipeline.h"

namespace Poly
{
	PassDeclarationColorBlendAttachment::PassDeclarationColorBlendAttachment(PassDeclarationGraphicsPipeline& pipeline)
	    : m_Pipeline(pipeline)
	{}

	PassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::BlendEnable(bool enable)
	{
		m_Current.BlendEnable = enable;
		return *this;
	}

	PassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::SrcColorBlendFactor(EBlendFactor factor)
	{
		m_Current.SrcColorBlendFactor = factor;
		return *this;
	}

	PassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::DstColorBlendFactor(EBlendFactor factor)
	{
		m_Current.DstColorBlendFactor = factor;
		return *this;
	}

	PassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::ColorBlendOp(EBlendOp op)
	{
		m_Current.ColorBlendOp = op;
		return *this;
	}

	PassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::SrcAlphaBlendFactor(EBlendFactor factor)
	{
		m_Current.SrcAlphaBlendFactor = factor;
		return *this;
	}

	PassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::DstAlphaBlendFactor(EBlendFactor factor)
	{
		m_Current.DstAlphaBlendFactor = factor;
		return *this;
	}

	PassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::AlphaBlendOp(EBlendOp op)
	{
		m_Current.AlphaBlendOp = op;
		return *this;
	}

	PassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::ColorWriteMask(FColorComponentFlag mask)
	{
		m_Current.ColorWriteMask = mask;
		return *this;
	}

	PassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::AddColorBlendAttachment()
	{
		m_Pipeline.CommitColorBlendAttachment(m_Current);
		m_Current = {};
		return *this;
	}

	PassDeclarationGraphicsPipeline& PassDeclarationColorBlendAttachment::FinishColorBlendAttachment()
	{
		m_Pipeline.CommitColorBlendAttachment(m_Current);
		m_Current = {};
		return m_Pipeline;
	}
} // namespace Poly
