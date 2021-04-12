#include "polypch.h"
#include "TestPass.h"
#include "../RenderContext.h"
#include "Platform/API/Sampler.h"
#include "Poly/Resources/ResourceLoader.h"

namespace Poly
{
	PassReflection TestPass::Reflect()
	{
		PassReflection reflection;
		// reflection.AddOutput("a");
		// reflection.SetFormat("a", EFormat::R8G8B8A8_UNORM);
		// reflection.SetBindPoint("a", FResourceBindPoint::COLOR_ATTACHMENT);

		// Vertex shader input
		reflection.AddInput("camera", 0, 0);
		reflection.SetBindPoint("camera", FResourceBindPoint::UNIFORM);

		// Fragment shader input
		reflection.AddInput("texture", 0, 1);
		reflection.SetBindPoint("texture", FResourceBindPoint::SAMPLER);
		reflection.SetSampler("texture", Sampler::GetDefaultLinearSampler());
		reflection.SetFormat("texture", EFormat::R8G8B8A8_UNORM);

		// Output
		reflection.AddOutput("out");
		reflection.SetBindPoint("out", FResourceBindPoint::COLOR_ATTACHMENT);
		reflection.SetFormat("out", EFormat::B8G8R8A8_UNORM);
		reflection.SetTextureSize("out", 0, 0); // 0 is invalid - defaults to default (window size)

		SetShader(FShaderStage::VERTEX, ResourceLoader::LoadShader("vert.glsl", FShaderStage::VERTEX));
		SetShader(FShaderStage::FRAGMENT, ResourceLoader::LoadShader("frag.glsl", FShaderStage::FRAGMENT));

		return reflection;
	}

	void TestPass::Execute(const RenderContext& context, const RenderData& renderData)
	{
		context.GetCommandBuffer()->DrawInstanced(3, 1, 0, 0);
	}
}