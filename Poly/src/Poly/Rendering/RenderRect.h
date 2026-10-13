#pragma once

namespace Poly
{
	// An area of a render target, in pixels with the origin in the top left corner
	struct RenderRect
	{
		int32  X      = 0;
		int32  Y      = 0;
		uint32 Width  = 0;
		uint32 Height = 0;
	};
} // namespace Poly
