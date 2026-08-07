#include "PassDeclarationVertexInput.h"

#include "PassDeclarationGraphicsPipeline.h"

namespace Poly
{
	PassDeclarationVertexInput::PassDeclarationVertexInput(PassDeclarationGraphicsPipeline& pipeline)
	    : m_Pipeline(pipeline)
	{}

	IPassDeclarationVertexInput& PassDeclarationVertexInput::Binding(uint32 binding)
	{
		m_Current.Binding = binding;
		return *this;
	}

	IPassDeclarationVertexInput& PassDeclarationVertexInput::Stride(uint32 stride)
	{
		m_Current.Stride = stride;
		return *this;
	}

	IPassDeclarationVertexInput& PassDeclarationVertexInput::VertexInputRate(EVertexInputRate rate)
	{
		m_Current.VertexInputRate = rate;
		return *this;
	}

	IPassDeclarationVertexInput& PassDeclarationVertexInput::Location(uint32 location)
	{
		m_Current.Location = location;
		return *this;
	}

	IPassDeclarationVertexInput& PassDeclarationVertexInput::Format(EFormat format)
	{
		m_Current.Format = format;
		return *this;
	}

	IPassDeclarationVertexInput& PassDeclarationVertexInput::Offset(uint32 offset)
	{
		m_Current.Offset = offset;
		return *this;
	}

	IPassDeclarationVertexInput& PassDeclarationVertexInput::AddVertexInput()
	{
		m_Pipeline.CommitVertexInput(m_Current);
		m_Current = {};
		return *this;
	}

	IPassDeclarationGraphicsPipeline& PassDeclarationVertexInput::FinishVertexInput()
	{
		m_Pipeline.CommitVertexInput(m_Current);
		m_Current = {};
		return m_Pipeline;
	}
} // namespace Poly
