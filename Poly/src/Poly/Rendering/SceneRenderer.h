#pragma once

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