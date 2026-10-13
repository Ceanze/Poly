#pragma once

namespace Poly
{
	/*
	 * Makes the entity a perspective camera, looking down its local -Z axis with +Y up.
	 */
	struct CameraComponent
	{
		float FieldOfView = 60.0f; // Vertical, in degrees
		float NearPlane   = 0.01f;
		float FarPlane    = 1000.0f;
		bool  Primary     = true; // If several cameras are primary, the first one found is used
	};
} // namespace Poly
