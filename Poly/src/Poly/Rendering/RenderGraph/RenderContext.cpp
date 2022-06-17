#include "polypch.h"
#include "RenderContext.h"

#include "Platform/API/CommandBuffer.h"

namespace Poly
{
	void RenderContext::SetCommandBuffer(CommandBuffer* pCommandBuffer)
	{
		m_pCommandBuffer = pCommandBuffer;
	}

	CommandBuffer* RenderContext::GetCommandBuffer() const
	{
		return m_pCommandBuffer;
	}
}