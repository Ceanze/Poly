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

		// Vertex Buffer
		reflection.AddInput("vertices", 1, 0);
		reflection.SetBindPoint("vertices", FResourceBindPoint::STORAGE);

		reflection.AddInput("instance", 1, 1);
		reflection.SetBindPoint("instance", FResourceBindPoint::STORAGE);

		// Fragment shader input
		reflection.AddInput("material", 2, 0);
		reflection.SetBindPoint("material", FResourceBindPoint::STORAGE);

		// Albedo
		reflection.AddInput("albedoTex", 2, 1);
		reflection.SetFormat("albedoTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("albedoTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("albedoTex", Sampler::GetDefaultLinearSampler());

		// Metallic
		reflection.AddInput("metallicTex", 2, 2);
		reflection.SetFormat("metallicTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("metallicTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("metallicTex", Sampler::GetDefaultLinearSampler());

		// Normal
		reflection.AddInput("normalTex", 2, 3);
		reflection.SetFormat("normalTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("normalTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("normalTex", Sampler::GetDefaultLinearSampler());

		// Roughness
		reflection.AddInput("roughnessTex", 2, 4);
		reflection.SetFormat("roughnessTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("roughnessTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("roughnessTex", Sampler::GetDefaultLinearSampler());

		// AO
		reflection.AddInput("aoTex", 2, 5);
		reflection.SetFormat("aoTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("aoTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("aoTex", Sampler::GetDefaultLinearSampler());

		// Combined
		reflection.AddInput("combinedTex", 2, 6);
		reflection.SetFormat("combinedTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("combinedTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("combinedTex", Sampler::GetDefaultLinearSampler());

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
		m_SceneRenderer.Execute(context);
	}
}
