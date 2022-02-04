#include "polypch.h"
#include "PBRPass.h"
#include "../RenderData.h"
#include "../RenderContext.h"
#include "Poly/Resources/ResourceLoader.h"

namespace Poly
{
	PassReflection PBRPass::Reflect()
	{
		PassReflection reflection;

		// Vertex shader input
		reflection.AddInput("camera", 0, 0);
		reflection.SetBindPoint("camera", FResourceBindPoint::UNIFORM);

		reflection.AddInput("lights", 0, 1);
		reflection.SetBindPoint("lights", FResourceBindPoint::STORAGE);

		reflection.AddSpecialInput("vertices", 1, 0, ESpecialInput::SCENE_VERTEX);
		reflection.AddSpecialInput("instanceBuffer", 1, 1, ESpecialInput::SCENE_INSTANCE);

		// Fragment shader input
		reflection.AddSpecialInput("materials", 2, 0, ESpecialInput::SCENE_MATERIAL);
		reflection.AddSpecialInput("textures", 2, 1, ESpecialInput::SCENE_TEXTURES); // SCENE_TEXTURE must be the last bound to a set - due to it using multiple bindings

		// Output
		reflection.AddOutput("out");
		reflection.SetBindPoint("out", FResourceBindPoint::COLOR_ATTACHMENT);
		reflection.SetFormat("out", EFormat::B8G8R8A8_UNORM);
		reflection.SetTextureSize("out", 0, 0); // 0 is invalid - defaults to default (window size)

		reflection.AddOutput("depth");
		reflection.SetBindPoint("depth", FResourceBindPoint::DEPTH_STENCIL);
		reflection.SetFormat("depth", EFormat::D24_UNORM_S8_UINT);

		SetShader(FShaderStage::VERTEX, ResourceLoader::LoadShader("shaders/pbr.vert", FShaderStage::VERTEX));
		SetShader(FShaderStage::FRAGMENT, ResourceLoader::LoadShader("shaders/pbr.frag", FShaderStage::FRAGMENT));

		return reflection;
	}

	void PBRPass::Execute(const RenderContext& context, const RenderData& renderData)
	{
		renderData.ExecuteScene(context);
	}
}