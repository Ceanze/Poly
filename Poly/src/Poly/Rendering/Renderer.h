#pragma once

namespace Poly
{
	class Resource;
	class SwapChain;
	class RenderGraphProgram;
	class Window;

	class Renderer
	{
	public:
		Renderer(Window* pWindow);
		~Renderer();

		static Ref<Renderer> Create(Window* pWindow);

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

	private:
		void CreateBackbufferResources();

		bool						m_HandleResize = false;
		Ref<SwapChain>				m_pSwapChain;
		Ref<RenderGraphProgram>		m_pRenderGraphProgram;
		Window*						m_pWindow = nullptr;
	};
}