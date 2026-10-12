#pragma once

#include "Poly/Core/Core.h"
#include "Poly/RenderGraph/RenderResourceTable.h"
#include "Poly/RenderGraph/RenderTransientResources.h"
#include "Poly/Rendering/RenderCamera.h"
#include "Poly/Rendering/RenderRect.h"

#include <string>

namespace Poly
{
	class World;

	struct RenderViewDesc
	{
		std::string DebugName;

		// Name of the view stage of the render plan the view is rendered by, see RenderPlanBuilder::AddViewStage()
		std::string Stage;

		// World to render, can be changed with RenderView::SetWorld()
		const World* pWorld = nullptr;

		// Texture to render to, invalid to render to the window. Can be changed with RenderView::SetTarget()
		TextureHandle Target;

		// Area of the target to render to, a size of zero covers the whole target. Can be changed with RenderView::SetRect()
		RenderRect Rect;
	};

	/*
	 * A point of view the renderer can render a world from, to the window or to a texture. What is rendered is
	 * decided by the view stage of the render plan the view belongs to.
	 *
	 * A view is persistent: it is created once with Renderer::CreateView() and rendered every frame it is
	 * submitted with Renderer::SubmitView(). What is set on it stays until it is changed. It owns the resources
	 * the renderer keeps per view (the camera buffer, depth), which are released with the view.
	 *
	 * Usage:
	 *
	 *   Ref<RenderView> pView = pRenderer->CreateView({.DebugName = "Game", .Stage = "Scene", .pWorld = &world});
	 *
	 *   // Every frame, the camera only has to be set when it has moved
	 *   pView->SetCamera(camera.GetRenderCamera());
	 *   pRenderer->SubmitView(pView);
	 */
	class RenderView
	{
	public:
		explicit RenderView(RenderViewDesc desc);
		~RenderView();
		CLASS_REMOVE_COPY(RenderView);

		/**
		 * Sets the world the view renders. A view without a world is not rendered.
		 */
		void SetWorld(const World* pWorld) { m_Desc.pWorld = pWorld; }

		/**
		 * Sets the camera the view is rendered from, e.g. CameraSystem::GetPrimaryCamera() of the world or the
		 * free camera of an editor. The view never looks for a camera on its own: until one is set it renders
		 * from the origin of the world, and it has to be set again when the camera moves.
		 */
		void SetCamera(const RenderCamera& camera) { m_Camera = camera; }

		/**
		 * Sets the texture the view renders to, an invalid handle renders to the window. The texture is owned by
		 * the caller and has to be created with FTextureUsage::COLOR_ATTACHMENT, and FTextureUsage::SAMPLED to be
		 * read afterwards. After being rendered to it is left in FResourceState::ShaderResource.
		 */
		void SetTarget(TextureHandle target) { m_Desc.Target = target; }

		/**
		 * Sets the area of the target the view renders to, a size of zero covers the whole target.
		 */
		void SetRect(const RenderRect& rect) { m_Desc.Rect = rect; }

		/**
		 * Resources provided to the passes rendering the view, for the resources registered with EResourceScope::View
		 * that the renderer does not provide itself. Providing Renderer::CAMERA_RESOURCE_NAME replaces the camera
		 * buffer of the view.
		 *
		 * Providing the target of another view makes this view read what that view renders: it is then
		 * rendered after that view, which has to belong to the same or an earlier view stage.
		 */
		RenderResourceTable&       GetResources() { return m_Resources; }
		const RenderResourceTable& GetResources() const { return m_Resources; }

		const std::string&  GetDebugName() const { return m_Desc.DebugName; }
		const std::string&  GetStage() const { return m_Desc.Stage; }
		const World*        GetWorld() const { return m_Desc.pWorld; }
		const RenderCamera& GetCamera() const { return m_Camera; }
		TextureHandle       GetTarget() const { return m_Desc.Target; }
		const RenderRect&   GetRect() const { return m_Desc.Rect; }

	private:
		friend class Renderer;

		// Uploads the camera and sets up the resources of the view for the frame about to be rendered.
		// The aspect ratio of the camera follows the rect of the view, or the target if it has no rect.
		void PrepareFrame(uint32 targetWidth, uint32 targetHeight);

		RenderViewDesc m_Desc;
		RenderCamera   m_Camera;

		RenderResourceTable m_Resources;

		// The resources of the view plus the ones the renderer provides, rebuilt every frame
		RenderResourceTable m_FrameResources;

		BufferHandle             m_CameraBufferHandle;
		RenderTransientResources m_Transients;
	};
} // namespace Poly
