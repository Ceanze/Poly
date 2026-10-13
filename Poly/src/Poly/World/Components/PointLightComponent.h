#pragma once

namespace Poly
{
	/*
	 * Makes the entity a point light, shining in all directions from its world position.
	 */
	struct PointLightComponent
	{
		glm::vec3 Color     = {1.0f, 1.0f, 1.0f};
		float     Intensity = 100.0f;
	};
} // namespace Poly
