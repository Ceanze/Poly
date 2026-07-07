#pragma once

namespace Poly
{
	class Camera;
	class Scene;
	class SwapChain;

	// Provided to RenderProgramInstance::Execute() once per call. Named RenderView (not
	// RenderContext) to avoid colliding with the unrelated Poly::RenderContext used by the
	// old render graph (Poly/Rendering/RenderGraph/RenderContext.h).
	struct RenderView
	{
		Camera*    pCamera = nullptr;
		Scene*     pScene  = nullptr;
		SwapChain* pTarget = nullptr;
		// viewport / clear options: TODO, not needed until a pass actually consumes them
	};
} // namespace Poly
