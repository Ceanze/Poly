#pragma once

namespace Poly
{
	// Structs defining the layout of the lights buffer: one GPULightsHeader followed by Count.x GPUPointLights.
	// This layout must match the layout in the shader code, currently declared in pbr_bindless.frag as LightsBuffer.
	struct GPULightsHeader
	{
		glm::vec4 Count; // x: number of point lights
	};

	struct GPUPointLight
	{
		glm::vec4 Color; // rgb: color scaled by intensity
		glm::vec4 Position;
	};
	static_assert(sizeof(GPULightsHeader) == 16, "GPULightsHeader must match pbr_bindless.frag's LightsBuffer layout");
	static_assert(sizeof(GPUPointLight) == 32, "GPUPointLight must match pbr_bindless.frag's PointLight layout");

} // namespace Poly
