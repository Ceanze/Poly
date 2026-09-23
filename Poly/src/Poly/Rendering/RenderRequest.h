#pragma once

namespace Poly
{
	class RenderResourceTable;
	class World;
	class Window;

	struct RenderRequest
	{
		const World*               pWorld         = nullptr;
		Window*                    pWindow        = nullptr;
		const RenderResourceTable* pViewResources = nullptr;
	};
} // namespace Poly
