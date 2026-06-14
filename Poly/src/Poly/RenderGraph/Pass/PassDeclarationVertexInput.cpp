#include "PassDeclarationVertexInput.h"

#include "PassDeclarationGraphicsPipeline.h"

namespace Poly
{
	PassDeclarationVertexInput::PassDeclarationVertexInput(PassDeclarationGraphicsPipeline& pipeline)
	    : m_Pipeline(pipeline)
	{}

	PassDeclarationVertexInput& PassDeclarationVertexInput::Binding(uint32 binding)
	{
		m_Current.Binding = binding;
		return *this;
	}

	PassDeclarationVertexInput& PassDeclarationVertexInput::Stride(uint32 stride)
	{
		m_Current.Stride = stride;
		return *this;
	}

	PassDeclarationVertexInput& PassDeclarationVertexInput::VertexInputRate(EVertexInputRate rate)
	{
		m_Current.VertexInputRate = rate;
		return *this;
	}

	PassDeclarationVertexInput& PassDeclarationVertexInput::Location(uint32 location)
	{
		m_Current.Location = location;
		return *this;
	}

	PassDeclarationVertexInput& PassDeclarationVertexInput::Format(EFormat format)
	{
		m_Current.Format = format;
		return *this;
	}

	PassDeclarationVertexInput& PassDeclarationVertexInput::Offset(uint32 offset)
	{
		m_Current.Offset = offset;
		return *this;
	}

	PassDeclarationVertexInput& PassDeclarationVertexInput::AddVertexInput()
	{
		m_Pipeline.CommitVertexInput(m_Current);
		m_Current = {};
		return *this;
	}

	PassDeclarationGraphicsPipeline& PassDeclarationVertexInput::FinishVertexInput()
	{
		m_Pipeline.CommitVertexInput(m_Current);
		m_Current = {};
		return m_Pipeline;
	}
} // namespace Poly
