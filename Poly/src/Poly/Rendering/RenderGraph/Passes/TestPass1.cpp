#include "polypch.h"
#include "TestPass1.h"

namespace Poly
{
	RenderPassReflection TestPass1::Reflect()
	{
		RenderPassReflection reflection;
		reflection.AddInput("a");
		reflection.SetFormat("a", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("a", FResourceBindPoint::SAMPLER);

		reflection.AddOutput("b");
		reflection.SetFormat("b", EFormat::B8G8R8A8_UNORM);
		reflection.SetBindPoint("b", FResourceBindPoint::COLOR_ATTACHMENT);

		return reflection;
	}
	
	void TestPass1::Execute()
	{

	}
}