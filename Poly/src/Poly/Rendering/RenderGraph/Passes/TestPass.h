#pragma once

#include "Poly/Rendering/RenderGraph/RenderPass.h"
#include "Poly/Rendering/SceneRenderer.h"

namespace Poly
{
	class TestPass : public RenderPass
	{
	public:
		TestPass() = default;
		~TestPass() = default;

		virtual PassReflection Reflect() override final;

		virtual void Update(const RenderContext& context) override final;

		virtual void Execute(const RenderContext& context, const RenderData& renderData) override final;

		static Ref<TestPass> Create() { return CreateRef<TestPass>(); }

	private:
		SceneRenderer m_SceneRenderer;
	};
}