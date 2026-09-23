#pragma once

namespace Poly
{
	class World;
	class Window;

	struct RenderRequest
	{
		const World* pWorld  = nullptr;
		Window*      pWindow = nullptr;
	};
} // namespace Poly
