#pragma once
#include "../RenderPass.h"

namespace Poly
{
	class TestPass1 : public RenderPass
	{
	public:
		TestPass1() = default;
		~TestPass1() = default;

		virtual PassReflection Reflect() override final;

		virtual void Execute(const RenderContext& context, const RenderData& renderData) override final;

		static Ref<TestPass1> Create() { return CreateRef<TestPass1>(); }

	private:

	};
}