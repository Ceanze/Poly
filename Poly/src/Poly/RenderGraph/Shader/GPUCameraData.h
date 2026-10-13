#pragma once

namespace Poly
{
	// Struct defining the layout of the camera buffer.
	// This layout must match the layout in the shader code, currently declared in pbr_bindless.slang as Camera.
	struct GPUCameraData
	{
		glm::mat4 ViewProjection;
		glm::vec4 Position;
	};
	static_assert(sizeof(GPUCameraData) == 80, "GPUCameraData must match the shaders' CameraBuffer layout");

} // namespace Poly
