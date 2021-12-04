#pragma once
#include "../RenderPass.h"

namespace Poly
{
	class PBRPass : public RenderPass
	{
	public:
		PBRPass() = default;
		~PBRPass() = default;

		virtual PassReflection Reflect() override final;

		virtual void Execute(const RenderContext& context, const RenderData& renderData) override final;

		static Ref<PBRPass> Create() { return CreateRef<PBRPass>(); }

	private:

	};
}