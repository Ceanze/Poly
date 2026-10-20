#pragma once

#include "Poly/RenderGraph/PipelineCache.h"
#include "Poly/RenderGraph/RenderProgramBuilder.h"
#include "Poly/RenderGraph/RenderResourceTable.h"
#include "Poly/RenderGraph/RenderTransientResources.h"
#include "Poly/Rendering/RenderView.h"

namespace Poly
{
	class SwapChain;
	class Window;
	class Event;
	class RenderPlan;
	class RenderCatalog;
	class RenderProgram;
	class RenderProgramInstance;

	/*
	 * Renders the views submitted each frame according to a render plan.
	 *
	 * The plan says what a frame can contain, the submitted views say what it does contain. Together they are
	 * compiled into a RenderProgram the first time a combination is seen, after that the program is reused.
	 *
	 * Usage:
	 *
	 *   // Init
	 *   RenderGraph graph; // uses the catalog of the renderer
	 *   pRenderer->SetRenderPlan(graph.BeginRenderPlan()...Build());
	 *   Ref<RenderView> pView = pRenderer->CreateView({.DebugName = "Game", .Stage = "Scene", .pWorld = &world});
	 *
	 *   // Every frame
	 *   pView->SetCamera(camera);
	 *   pRenderer->SubmitView(pView);
	 */
	class Renderer
	{
	public:
		// Uniform buffer (GPUCameraData) of the camera a view is rendered from, provided by the renderer every frame.
		// A view providing the same name through RenderView::GetResources() takes precedence
		static constexpr const char* CAMERA_RESOURCE_NAME = "view.camera";

		Renderer();
		~Renderer();

		static Unique<Renderer> Create();

		/**
		 * Creates a view. The view lives for as long as the caller holds on to it, see RenderView.
		 * @param desc - description of the view
		 */
		Ref<RenderView> CreateView(const RenderViewDesc& desc);

		/**
		 * Submits a view to be rendered during the next Render() call. Submits only live for a single frame, so a
		 * view has to be submitted every frame it should be rendered, and at most once per frame.
		 * A view that has no world or whose stage is not part of the render plan is not rendered.
		 * @param pView - the view to render
		 */
		void SubmitView(const Ref<RenderView>& pView);

		/**
		 * Gets the catalog render plans are built from, holding what the engine registers. Applications register their
		 * own passes, features and resources in it too.
		 */
		const Ref<RenderCatalog>& GetRenderCatalog() const { return m_pCatalog; }

		/**
		 * Gets the resources provided to the passes of every view and frame feature, for the resources
		 * registered with EResourceScope::Global.
		 */
		RenderResourceTable&       GetGlobalResources() { return m_GlobalResources; }
		const RenderResourceTable& GetGlobalResources() const { return m_GlobalResources; }

		/**
		 * Sets the render plan to render frames with, replacing the current one.
		 * @param pRenderPlan - plan built with RenderGraph::BeginRenderPlan()
		 */
		void SetRenderPlan(Ref<RenderPlan> pRenderPlan);

		/**
		 * Adds a window to be rendered when Render() is called.
		 * Only the first window added is rendered to for now.
		 * @param pWindow - Pointer to the window to add
		 */
		void AddWindow(Window* pWindow);

		/**
		 * Removes a window from being rendered
		 * @param pWindow - Pointer to the window to remove
		 */
		void RemoveWindow(Window* pWindow);

		/**
		 * Renders the frame features of the render plan and the views submitted via SubmitView() this frame.
		 */
		void Render();

		void OnEvent(Event& event);

	private:
		struct WindowContext
		{
			Window*        pWindow;
			Ref<SwapChain> pSwapChain;
		};

		// A compiled program together with what is needed to execute it, kept so that a combination of views
		// that has been rendered before does not have to be compiled again
		struct CachedProgram
		{
			ProgramKey                    Key;
			Ref<RenderProgram>            pProgram;
			Unique<RenderProgramInstance> pInstance;
			uint64                        LastUsedFrame = 0;
		};

		// The views of the frame that can be rendered, in the order they are rendered: by view stage, with a view
		// reading the target of another view of the same stage after that view, otherwise in submit order
		std::vector<RenderView*> OrderSubmittedViews() const;

		// Describes the structure of the frame, see ProgramKey
		ProgramKey BuildProgramKey(const std::vector<RenderView*>& views) const;

		// Gets the program for the key from the cache, compiling it first if it is not there
		CachedProgram& GetOrBuildProgram(const ProgramKey& key);

		// Destroys all cached programs and pipelines, once the GPU no longer uses them
		void ClearPrograms();

		// Limit of the program cache, the least recently used program is dropped when it is exceeded
		static constexpr size_t MAX_CACHED_PROGRAMS = 16;

		std::vector<WindowContext> m_Windows;

		Ref<RenderCatalog> m_pCatalog;

		Ref<RenderPlan> m_pRenderPlan;

		// Declared before the programs, whose instances refer to it
		PipelineCache              m_PipelineCache;
		std::vector<CachedProgram> m_ProgramCache;
		uint64                     m_FrameCounter = 0;

		std::vector<Ref<RenderView>> m_SubmittedViews;
		RenderResourceTable          m_GlobalResources;

		// Owner of the resources render programs allocate for frame features, the views own their own
		RenderTransientResources m_FrameTransients;
	};
} // namespace Poly
