#pragma once

#include "Poly/Core/Handle.h"
#include "Poly/Rendering/RenderRect.h"

namespace Poly
{
	class RenderResourceTable;
	class RenderTransientResources;
	class TextureView;
	class World;

	using TextureHandle = Handle<struct TextureHandleTag>;

	/*
	 * What a pass sees of the view it is recorded for. One exists per view slot of a render program for
	 * the duration of RenderProgramInstance::Execute(): slot 0 is the frame itself (the window, without a
	 * world), used by the passes of frame features, the following slots are the submitted views.
	 */
	struct RenderViewContext
	{
		const RenderResourceTable* pGlobalResources = nullptr;
		const World*               pWorld           = nullptr;
		const RenderResourceTable* pViewResources   = nullptr;

		// The color target of the view, either the backbuffer of the window or the offscreen target of the view
		TextureView* pTarget = nullptr;

		// The offscreen target of the view, invalid when rendering to the window
		TextureHandle TargetHandle;

		// The area of the target the view renders to, a size of zero covers the whole target
		RenderRect Rect;

		// Owner of the resources the render program allocates for this view (e.g. depth)
		RenderTransientResources* pTransients = nullptr;
	};
} // namespace Poly
