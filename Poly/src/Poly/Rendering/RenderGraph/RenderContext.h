#pragma once

namespace Poly
{
	class CommandBuffer;

	class RenderContext
	{
	public:
		RenderContext() = default;
		~RenderContext() = default;

		void SetCommandBuffer(CommandBuffer* pCommandBuffer);

		const CommandBuffer* GetCommandBuffer() const;

		static Ref<RenderContext> Create();

	private:
		CommandBuffer*	m_pCommandBuffer	= nullptr;
	};
}