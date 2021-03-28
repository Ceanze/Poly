#pragma once

namespace Poly
{
	class SwapChain;
	class RenderGraphProgram;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		static Ref<Renderer> Create();

		/**
		 * Sets the currently used render graph program
		 * @param pRenderGraphProgram
		 */
		void SetRenderGraph(Ref<RenderGraphProgram> pRenderGraphProgram);

		/**
		 * Renders the with the current render graph
		 * @param [FUTURE PURPOSE - Scene to render]
		 */
		void Render();

		// void ResizeCallback()

	private:
		Ref<SwapChain> m_pSwapChain;
		Ref<RenderGraphProgram> m_pRenderGraphProgram;
	};
}