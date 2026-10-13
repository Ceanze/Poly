#pragma once

#include "Platform/API/GraphicsPipeline.h"
#include "Poly/RenderGraph/BufferRange.h"
#include "Vertex.h"

#include <glm/glm.hpp>

namespace Poly
{
	// The alignas(16) on TexCoord forces trailing padding so this struct's size (and therefore its
	// per-element stride when indexed as an array via buffer_reference) is 64 bytes, matching
	// pbr_bindless.slang's BDA-side `vec4 TexCoord` - only .xy of that vec4 is ever read, so the extra
	// 8 bytes of padding this struct doesn't otherwise use are simply never touched by the shader.
	static_assert(sizeof(Vertex) == 64, "Vertex must keep a 64-byte stride to match pbr_bindless.slang's Vertex layout");

	struct MeshRange
	{
		BufferRange Vertices;
		BufferRange Indices;
	};
} // namespace Poly
