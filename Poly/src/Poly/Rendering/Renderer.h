#pragma once

#include "FrameContext.h"

namespace Poly
{
	class HeadlessFrameContext;
	class RenderGraphProgram;
	class Window;
	class Event;

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
		 * Renders with the current render graph
		 * @param [FUTURE PURPOSE - Scene to render]
		 */
		void Render();

		void OnEvent(Event& event);

		/**
		 * TODO: Move to be enabled in construction, and have it be a application setting - it cannot change during runtime
		 * Enables headless rendering mode. Creates custom backbuffer images that
		 * serve as render targets when no window/swapchain is present.
		 * @param width  - Width of the headless backbuffer images
		 * @param height - Height of the headless backbuffer images
		 */
		void EnableHeadless(uint32 width, uint32 height);

		/**
		 * TODO: Move to be enabled in construction, and have it be a application setting - it cannot change during runtime
		 * Disables headless rendering mode and releases the custom backbuffer images.
		 * Waits for GPU idle before releasing resources.
		 */
		void DisableHeadless();

		/**
		 * Returns one of the headless backbuffer textures for readback / frame sharing.
		 * @param index - Backbuffer index in [0, HEADLESS_BUFFER_COUNT)
		 * @return Texture ref, or nullptr if headless mode is not active
		 */
		Ref<Texture> GetHeadlessTexture(uint32 index) const;

	private:
		void CreateBackbufferResources(FrameContext& ctx);

		bool                           m_HandleResize = false;
		Ref<RenderGraphProgram>        m_pRenderGraphProgram;
		std::vector<Ref<FrameContext>> m_FrameContexts;
	};
} // namespace Poly
