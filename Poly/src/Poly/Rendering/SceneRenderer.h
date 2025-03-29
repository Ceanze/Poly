#pragma once

#include "Poly/Scene/Scene.h"
#include "Poly/Model/Model.h"
#include "Poly/Model/Material.h"

#include "Poly/Rendering/RenderGraph/PassReflection.h"

namespace Poly
{
	class RenderContext;

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;
		~SceneRenderer() = default;

		void Execute(const RenderContext& context);

	private:
		void Render(const RenderContext& context);
	};

}