#pragma once

namespace Poly
{
	class Camera;
	class RenderResourceTable;
	class Scene;
	class TextureView;
	class World;

	// Provided to RenderProgramInstance::Execute() once per call
	struct RenderView
	{
		const RenderResourceTable* pGlobalResources = nullptr;
		const World*               pWorld           = nullptr;
		const RenderResourceTable* pViewResources   = nullptr;
		TextureView*               pTarget          = nullptr;
		// viewport / clear options: TODO, not needed until a pass actually consumes them
	};
} // namespace Poly
