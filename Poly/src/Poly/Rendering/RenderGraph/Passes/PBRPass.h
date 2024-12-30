#pragma once

#include "Poly/Rendering/RenderGraph/RenderPass.h"
#include "Poly/Rendering/SceneRenderer.h"

namespace Poly
{
	class PBRPass : public RenderPass
	{
	public:
		PBRPass() = default;
		~PBRPass() = default;

		virtual void Compile() override final;

		virtual PassReflection Reflect() override final;

		virtual void Execute(const RenderContext& context, const RenderData& renderData) override final;

		static Ref<PBRPass> Create() { return CreateRef<PBRPass>(); }

	private:
		SceneRenderer m_SceneRenderer;
	};
}