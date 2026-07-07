#pragma once

namespace Poly
{
	class Resource;
	class SwapChain;
	class RenderGraphProgram;
	class Window;
	class Event;
	class RenderProgram;
	class RenderProgramInstance;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		static Unique<Renderer> Create();

		/**
		 * Sets the currently used render graph program
		 * @param pRenderGraphProgram
		 */
		void SetRenderGraph(Ref<RenderGraphProgram> pRenderGraphProgram);

		/**
		 * Sets the render program to use once it is safe to swap out the currently active one.
		 * Constructs a RenderProgramInstance and queues it; it becomes active at the start of
		 * the next Render() call.
		 * @param pRenderProgram
		 */
		void SetRenderProgram(std::unique_ptr<RenderProgram> pRenderProgram);

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

		void OnEvent(Event& event);

	private:
		struct WindowContext
		{
			Window*        pWindow;
			Ref<SwapChain> pSwapChain;
		};

		void CreateBackbufferResources(const WindowContext& windowCtx);

		// Swaps in the queued RenderProgramInstance, if one is waiting. Called at a point in
		// the frame where it's safe to retire the previously active instance (see
		// plans/render_graph.md, "Render Program"). Real GPU-idle gating is future work.
		void SwapRenderProgramInstanceIfQueued();

		bool                          m_HandleResize = false;
		Ref<RenderGraphProgram>       m_pRenderGraphProgram;
		std::vector<WindowContext>    m_Windows;

		Unique<RenderProgramInstance> m_pActiveRenderProgramInstance;
		Unique<RenderProgramInstance> m_pQueuedRenderProgramInstance;
	};
} // namespace Poly