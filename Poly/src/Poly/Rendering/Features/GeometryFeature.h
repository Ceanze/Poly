#pragma once

#include "Poly/Core/Core.h"

namespace Poly
{
	class RenderCatalog;

	/*
	 * The built-in feature drawing the renderable entities of a world with the PBR shaders.
	 * Draws the batches of the world's RenderSystem, so the world being rendered needs one.
	 *
	 * The camera is provided by the Renderer (Renderer::CAMERA_RESOURCE_NAME) and the lights by the LightSystem,
	 * so the world needs one of those too.
	 *
	 * Usage:
	 *
	 *   // Before building the render plan
	 *   GeometryFeature::Register(catalog);
	 *   graph.BeginRenderPlan().AddViewStage("Scene").AddFeature(GeometryFeature::FEATURE_NAME).EndStage()...Build();
	 */
	class GeometryFeature
	{
	public:
		CLASS_STATIC(GeometryFeature);

		static constexpr const char* FEATURE_NAME = "geometry";

		// Draws the batches of the RenderSystem of the world being rendered
		static constexpr const char* SCENE_RASTER_EXECUTOR = "SceneRaster";

		/*
		 * Registers the camera resource, the SCENE_RASTER_EXECUTOR executor, the "pbr" pass and the FEATURE_NAME feature in the catalog.
		 * @param catalog - catalog the render plan will be built from
		 */
		static void Register(RenderCatalog& catalog);
	};
} // namespace Poly
