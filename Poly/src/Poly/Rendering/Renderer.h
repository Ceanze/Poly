#pragma once

namespace Poly
{
	class Resource;
	class SwapChain;
	class RenderGraphProgram;
	class Window;
	class Event;
	class Scene;
	class RenderProgram;
	class RenderProgramInstance;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		static Unique<Renderer> Create();

		/**
		 * Sets the scene to render (currently the same scene for all windows/render instances)
		 * @param pScene - Scene to render
		 */
		void SetScene(Ref<Scene> pScene);

		/**
		 * Sets the currently used render graph program
		 * @param pRenderGraphProgram
		 */
		void SetRenderGraph(Ref<RenderGraphProgram> pRenderGraphProgram);

		/**
		 * Sets the render program to use once it is safe to swap out the currently active one.
		 * Queues the compiled program; a RenderProgramInstance is constructed for
		 * each window from it at the start of the next Render() call.
		 * @param pRenderProgram
		 */
		void SetRenderProgram(Ref<RenderProgram> pRenderProgram);

		/**
		 * Gets the RenderProgramInstance actively executing the current RenderProgram for a window.
		 * Returns nullptr until a queued RenderProgram set via SetRenderProgram() has actually been
		 * swapped in (see SwapRenderProgramIfQueued(), which runs at the start of the next Render()
		 * call) - so this can still return nullptr on the very first frame after SetRenderProgram().
		 * @param pWindow - window to get the instance for; nullptr uses the first added window.
		 */
		RenderProgramInstance* GetRenderProgramInstance(Window* pWindow = nullptr) const;

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
			Window*                       pWindow;
			Ref<SwapChain>                pSwapChain;
			Unique<RenderProgramInstance> pRenderProgramInstance;
		};

		void CreateBackbufferResources(const WindowContext& windowCtx);

		// Swaps in the queued RenderProgram, if one is waiting, by constructing a fresh
		// RenderProgramInstance per window from it. Called at a point in the frame where it's
		// safe to retire the previously active instances (see plans/render_graph.md, "Render
		// Program"). Real GPU-idle gating is future work.
		void SwapRenderProgramIfQueued();

		bool                       m_HandleResize = false;
		Ref<RenderGraphProgram>    m_pRenderGraphProgram;
		std::vector<WindowContext> m_Windows;

		Ref<Scene>         m_pScene;
		Ref<RenderProgram> m_pActiveRenderProgram;
		Ref<RenderProgram> m_pQueuedRenderProgram;
	};
} // namespace Poly