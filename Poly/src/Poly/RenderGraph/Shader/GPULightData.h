#pragma once

namespace Poly
{
	// Structs defining the layout of the lights buffer: one GPULightsHeader followed by Count.x GPUPointLights.
	// This layout must match the layout in the shader code, currently declared in pbr_bindless.slang as LightsHeader and PointLight.
	struct GPULightsHeader
	{
		glm::vec4 Count; // x: number of point lights
	};

	struct GPUPointLight
	{
		glm::vec4 Color; // rgb: color scaled by intensity
		glm::vec4 Position;
	};
	static_assert(sizeof(GPULightsHeader) == 16, "GPULightsHeader must match pbr_bindless.slang's LightsHeader layout");
	static_assert(sizeof(GPUPointLight) == 32, "GPUPointLight must match pbr_bindless.slang's PointLight layout");

} // namespace Poly
