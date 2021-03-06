#pragma once

#include "Platform/API/CommandBuffer.h" // CommandBuffer is important for the interface of RenderContext, included here for ease of use

namespace Poly
{
	class CommandBuffer;

	class RenderContext
	{
	public:
		RenderContext() = default;
		~RenderContext() = default;

		CommandBuffer* GetCommandBuffer() const;

		static Ref<RenderContext> Create();

	private:
		friend class RenderGraphProgram;
		void SetCommandBuffer(CommandBuffer* pCommandBuffer);

		CommandBuffer*	m_pCommandBuffer	= nullptr;
	};
}