#include "PassDeclarationColorBlendAttachment.h"

#include "PassDeclarationGraphicsPipeline.h"

namespace Poly
{
	PassDeclarationColorBlendAttachment::PassDeclarationColorBlendAttachment(PassDeclarationGraphicsPipeline& pipeline)
	    : m_Pipeline(pipeline)
	{}

	IPassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::BlendEnable(bool enable)
	{
		m_Current.BlendEnable = enable;
		return *this;
	}

	IPassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::SrcColorBlendFactor(EBlendFactor factor)
	{
		m_Current.SrcColorBlendFactor = factor;
		return *this;
	}

	IPassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::DstColorBlendFactor(EBlendFactor factor)
	{
		m_Current.DstColorBlendFactor = factor;
		return *this;
	}

	IPassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::ColorBlendOp(EBlendOp op)
	{
		m_Current.ColorBlendOp = op;
		return *this;
	}

	IPassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::SrcAlphaBlendFactor(EBlendFactor factor)
	{
		m_Current.SrcAlphaBlendFactor = factor;
		return *this;
	}

	IPassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::DstAlphaBlendFactor(EBlendFactor factor)
	{
		m_Current.DstAlphaBlendFactor = factor;
		return *this;
	}

	IPassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::AlphaBlendOp(EBlendOp op)
	{
		m_Current.AlphaBlendOp = op;
		return *this;
	}

	IPassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::ColorWriteMask(FColorComponentFlag mask)
	{
		m_Current.ColorWriteMask = mask;
		return *this;
	}

	IPassDeclarationColorBlendAttachment& PassDeclarationColorBlendAttachment::AddColorBlendAttachment()
	{
		m_Pipeline.CommitColorBlendAttachment(m_Current);
		m_Current = {};
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationColorBlendAttachment::FinishColorBlendAttachment()
	{
		m_Pipeline.CommitColorBlendAttachment(m_Current);
		m_Current = {};
		return m_Pipeline;
	}
} // namespace Poly
