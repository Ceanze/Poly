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
		reflection.SetFormat("out", EFormat::R8G8B8A8_UNORM);
		reflection.SetTextureSize("out", 0, 0); // 0 is invalid - defaults to default (window size)

		reflection.AddOutput("depth");
		reflection.SetBindPoint("depth", FResourceBindPoint::DEPTH_STENCIL);
		reflection.SetFormat("depth", EFormat::DEPTH_STENCIL);

		SetShader(FShaderStage::VERTEX, ResourceLoader::LoadShader("shaders/vert.glsl", FShaderStage::VERTEX));
		SetShader(FShaderStage::FRAGMENT, ResourceLoader::LoadShader("shaders/frag.glsl", FShaderStage::FRAGMENT));

		return reflection;
	}

	void TestPass::Execute(const RenderContext& context, const RenderData& renderData)
	{
		m_SceneRenderer.Execute(context, renderData.GetScene(), renderData.GetPassReflection(), context.GetImageIndex());
	}
}
