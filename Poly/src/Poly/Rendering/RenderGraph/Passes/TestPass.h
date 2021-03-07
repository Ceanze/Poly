#pragma once
#include "../RenderPass.h"

namespace Poly
{
	class TestPass : public RenderPass
	{
	public:
		TestPass() = default;
		~TestPass() = default;

		virtual RenderPassReflection Reflect() override final;

		virtual void Execute() override final;

		static Ref<TestPass> Create() { return CreateRef<TestPass>(); }

	private:

	};
}