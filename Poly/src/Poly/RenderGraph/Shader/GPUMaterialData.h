#pragma once

#include "Poly/Model/Material.h"

namespace Poly
{
	// Struct defining the layout of instance data in the instance buffer.
	// This layout must match the layout in the shader code, currently declared in common/bindless.glsl as InstanceData.
	struct GPUMaterialData
	{
		MaterialValues Values;
		uint32         TextureAlbedoIndex;
		uint32         TextureMetallicIndex;
		uint32         TextureNormalIndex;
		uint32         TextureRoughnessIndex;
		uint32         TextureAmbientOcclusionIndex;
		uint32         TextureCombinedIndex;
		uint32         _Pad[2];
	};
	static_assert(offsetof(GPUMaterialData, TextureAlbedoIndex) == 32, "GPUMaterialData::TextureIndices must sit right after Values");
	static_assert(sizeof(GPUMaterialData) == 64, "GPUMaterialData must match pbr_bindless.frag's MaterialValues layout (64-byte std430 stride)");
	static_assert(sizeof(MaterialValues) == 32, "GPUMaterialData below assumes MaterialValues is exactly 32 bytes with no internal padding");
} // namespace Poly