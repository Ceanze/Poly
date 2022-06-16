#include "polypch.h"
#include "PBRPass.h"
#include "../RenderData.h"
#include "../RenderContext.h"
#include "Poly/Resources/ResourceLoader.h"

namespace Poly
{
	void PBRPass::Compile()
	{
		SetAutoBind(0, true);
		SetAutoBind(1, false);
		SetAutoBind(2, false);
	}

	PassReflection PBRPass::Reflect()
	{
		PassReflection reflection;

		// Vertex shader input
		reflection.AddInput("camera", 0, 0);
		reflection.SetBindPoint("camera", FResourceBindPoint::UNIFORM);

		reflection.AddInput("lights", 0, 1);
		reflection.SetBindPoint("lights", FResourceBindPoint::STORAGE);

		reflection.AddSceneInput("vertices", 1, 0, ESceneBinding::VERTEX);
		reflection.AddSceneInput("instanceBuffer", 1, 1, ESceneBinding::INSTANCE);

		// Fragment shader input
		reflection.AddSceneInput("materials", 2, 0, ESceneBinding::MATERIAL);
		reflection.AddSceneInput("albedoTex", 2, 1, ESceneBinding::TEXTURE_ALBEDO);
		reflection.AddSceneInput("metallicTex", 2, 2, ESceneBinding::TEXTURE_METALLIC);
		reflection.AddSceneInput("normalTex", 2, 3, ESceneBinding::TEXTURE_NORMAL);
		reflection.AddSceneInput("roughnessTex", 2, 4, ESceneBinding::TEXTURE_ROUGHNESS);
		reflection.AddSceneInput("aoTex", 2, 5, ESceneBinding::TEXTURE_AO);
		reflection.AddSceneInput("combinedTex", 2, 6, ESceneBinding::TEXTURE_COMBINED);

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