#pragma once

namespace Poly
{
	// Struct defining the layout of instance data in the instance buffer.
	// This layout must match the layout in the shader code, currently declared in common/bindless.slang as InstanceData.
	struct GPUInstanceData
	{
		glm::mat4 Transform;
		uint32    MaterialIndex;
		uint32    _Pad[3];
	};
	static_assert(offsetof(GPUInstanceData, MaterialIndex) == 64, "GPUInstanceData::MaterialIndex must sit right after Transform");
	static_assert(sizeof(GPUInstanceData) == 80, "GPUInstanceData must match common/bindless.slang's GPUInstanceData layout (80-byte std430 stride)");

} // namespace Poly
