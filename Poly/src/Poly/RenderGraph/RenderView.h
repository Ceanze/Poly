#pragma once

namespace Poly
{
	class Camera;
	class Scene;
	class TextureView;

	// Provided to RenderProgramInstance::Execute() once per call
	struct RenderView
	{
		Scene*       pScene  = nullptr;
		TextureView* pTarget = nullptr;
		// viewport / clear options: TODO, not needed until a pass actually consumes them
	};
} // namespace Poly
