#pragma once

#include "Poly/Core/Core.h"

namespace Poly
{
	class RenderCatalog;

	/*
	 * The built-in feature drawing the renderable entities of a world with the PBR shaders.
	 * Draws the batches of the world's RenderSystem, so the world being rendered needs one.
	 *
	 * The camera and lights are not part of the world yet, the application provides their buffers
	 * through a RenderResourceTable under CAMERA_RESOURCE_NAME and LIGHTS_RESOURCE_NAME.
	 *
	 * Usage:
	 *
	 *   // Before building the render program
	 *   GeometryFeature::Register(catalog);
	 *   graph.Begin().AddFeature(GeometryFeature::FEATURE_NAME)...Build();
	 *
	 *   viewResources.Set(GeometryFeature::CAMERA_RESOURCE_NAME, cameraBufferHandle);
	 *   viewResources.Set(GeometryFeature::LIGHTS_RESOURCE_NAME, lightsBufferHandle);
	 */
	class GeometryFeature
	{
	public:
		CLASS_STATIC(GeometryFeature);

		static constexpr const char* FEATURE_NAME = "geometry";

		static constexpr const char* CAMERA_RESOURCE_NAME = "Camera";
		static constexpr const char* LIGHTS_RESOURCE_NAME = "Lights";

		/*
		 * Registers the camera and lights resources, the "pbr" pass and the FEATURE_NAME feature in the catalog.
		 * @param catalog - catalog the render program will be built from
		 */
		static void Register(RenderCatalog& catalog);
	};
} // namespace Poly
