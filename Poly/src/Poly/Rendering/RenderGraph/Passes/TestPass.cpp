#include "polypch.h"
#include "TestPass.h"
#include "../RenderContext.h"
#include "../RenderData.h"
#include "Platform/API/Sampler.h"
#include "Poly/Resources/ResourceLoader.h"

namespace Poly
{
	PassReflection TestPass::Reflect()
	{
		PassReflection reflection;

		// Vertex shader input
		reflection.AddInput("camera", 0, 0);
		reflection.SetBindPoint("camera", FResourceBindPoint::UNIFORM);

		reflection.AddSpecialInput("vertices", 1, 0, ESpecialInput::SCENE_VERTEX);
		reflection.AddSpecialInput("instanceBuffer", 1, 1, ESpecialInput::SCENE_INSTANCE);

		// Fragment shader input
		reflection.AddSpecialInput("materials", 2, 0, ESpecialInput::SCENE_MATERIAL);
		reflection.AddSpecialInput("textures", 2, 1, ESpecialInput::SCENE_TEXTURES); // SCENE_TEXTURE must be the last bound to a set - due to it using multiple bindings

		// Output
		reflection.AddOutput("out");
		reflection.SetBindPoint("out", FResourceBindPoint::COLOR_ATTACHMENT);
		reflection.SetFormat("out", EFormat::R8G8B8A8_UNORM);
		reflection.SetTextureSize("out", 0, 0); // 0 is invalid - defaults to default (window size)

		reflection.AddOutput("depth");
		reflection.SetBindPoint("depth", FResourceBindPoint::DEPTH_STENCIL);
		reflection.SetFormat("depth", EFormat::D24_UNORM_S8_UINT);

		SetShader(FShaderStage::VERTEX, ResourceLoader::LoadShader("vert.glsl", FShaderStage::VERTEX));
		SetShader(FShaderStage::FRAGMENT, ResourceLoader::LoadShader("frag.glsl", FShaderStage::FRAGMENT));

		return reflection;
	}

	void TestPass::Execute(const RenderContext& context, const RenderData& renderData)
	{
		renderData.ExecuteScene(context);
	}
}