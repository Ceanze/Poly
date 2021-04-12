#include "polypch.h"
#include "TestPass1.h"

namespace Poly
{
	PassReflection TestPass1::Reflect()
	{
		PassReflection reflection;
		reflection.AddInput("a", 0, 0);
		reflection.SetFormat("a", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("a", FResourceBindPoint::SAMPLER);

		reflection.AddOutput("b");
		reflection.SetFormat("b", EFormat::B8G8R8A8_UNORM);
		reflection.SetBindPoint("b", FResourceBindPoint::COLOR_ATTACHMENT);

		return reflection;
	}

	void TestPass1::Execute(const RenderContext& context, const RenderData& renderData)
	{

	}
}