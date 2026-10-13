#pragma once

namespace Poly
{
	/*
	 * A camera as the renderer sees it: where it is and how it projects. The aspect ratio is not part of it,
	 * the renderer takes it from the target being rendered to.
	 */
	struct RenderCamera
	{
		glm::mat4 View        = glm::mat4(1.0f);
		glm::vec3 Position    = {0.0f, 0.0f, 0.0f};
		float     FieldOfView = glm::radians(60.0f); // Vertical, in radians
		float     NearPlane   = 0.01f;
		float     FarPlane    = 1000.0f;
	};
} // namespace Poly
