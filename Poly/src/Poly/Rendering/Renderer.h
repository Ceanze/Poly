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
		Renderer();
		~Renderer();

		static Ref<Renderer> Create();

		/**
		 * Sets the currently used render graph program
		 * @param pRenderGraphProgram
		 */
		void SetRenderGraph(Ref<RenderGraphProgram> pRenderGraphProgram);

		/**
		* Adds a window to be rendered when Render() is called
		* @param pWindow - Pointer to the window to add
		*/
		void AddWindow(Window* pWindow);

		/**
		* Removes a window from being rendered
		* @param pWindow - Pointer to the window to remove
		*/
		void RemoveWindow(Window* pWindow);

		/**
		 * Renders the with the current render graph
		 * @param [FUTURE PURPOSE - Scene to render]
		 */
		void Render();

	private:
		struct WindowContext
		{
			Window* pWindow;
			Ref<SwapChain> pSwapChain;
		};

		void CreateBackbufferResources(const WindowContext& windowCtx);

		bool						m_HandleResize = false;
		Ref<RenderGraphProgram>		m_pRenderGraphProgram;
		std::vector<WindowContext>	m_Windows;
	};
}