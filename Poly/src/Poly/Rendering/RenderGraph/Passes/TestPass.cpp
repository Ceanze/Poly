#include "polypch.h"
#include "TestPass.h"

namespace Poly
{
	PassReflection TestPass::Reflect()
	{
		PassReflection reflection;
		reflection.AddOutput("a");
		reflection.SetFormat("a", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("a", FResourceBindPoint::COLOR_ATTACHMENT);

		return reflection;
	}

	void TestPass::Execute(const RenderContext& context, const RenderData& renderData)
	{

	}
}