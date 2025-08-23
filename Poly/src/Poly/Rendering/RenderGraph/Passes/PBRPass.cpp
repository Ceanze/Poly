#include "polypch.h"
#include "PBRPass.h"
#include "../RenderData.h"
#include "../RenderContext.h"
#include "Poly/Resources/ResourceLoader.h"
#include "Poly/Resources/Shader/ShaderManager.h"

#include "Platform/API/Sampler.h"

namespace Poly
{
	void PBRPass::Compile()
	{
		ToggleInstancedSceneRendering(true);
	}

	PassReflection PBRPass::Reflect()
	{
		PassReflection reflection;
		//ShaderReflector reflector(this);
		//reflector.Reflect("pbt.vert");
		//reflector.Reflect("pbt.frag");
		//return reflector.GetReflection();

		// Vertex shader input
		reflection.AddInput("camera", 0, 0);
		reflection.SetBindPoint("camera", FResourceBindPoint::UNIFORM);

		reflection.AddInput("lights", 0, 1);
		reflection.SetBindPoint("lights", FResourceBindPoint::STORAGE);
		
		// Vertex Buffer
		reflection.AddInput("vertices", 4, 0);
		reflection.SetBindPoint("vertices", FResourceBindPoint::STORAGE);

		reflection.AddInput("instance", 1, 0);
		reflection.SetBindPoint("instance", FResourceBindPoint::STORAGE);

		// Fragment shader input
		reflection.AddInput("material", 2, 0);
		reflection.SetBindPoint("material", FResourceBindPoint::STORAGE);

		// Albedo
		reflection.AddInput("albedoTex", 3, 0);
		reflection.SetFormat("albedoTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("albedoTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("albedoTex", Sampler::GetDefaultLinearSampler());

		// Metallic
		reflection.AddInput("metallicTex", 3, 1);
		reflection.SetFormat("metallicTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("metallicTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("metallicTex", Sampler::GetDefaultLinearSampler());

		// Normal
		reflection.AddInput("normalTex", 3, 2);
		reflection.SetFormat("normalTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("normalTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("normalTex", Sampler::GetDefaultLinearSampler());

		// Roughness
		reflection.AddInput("roughnessTex", 3, 3);
		reflection.SetFormat("roughnessTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("roughnessTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("roughnessTex", Sampler::GetDefaultLinearSampler());

		// AO
		reflection.AddInput("aoTex", 3, 4);
		reflection.SetFormat("aoTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("aoTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("aoTex", Sampler::GetDefaultLinearSampler());

		// Combined
		reflection.AddInput("combinedTex", 3, 5);
		reflection.SetFormat("combinedTex", EFormat::R8G8B8A8_UNORM);
		reflection.SetBindPoint("combinedTex", FResourceBindPoint::SAMPLER | FResourceBindPoint::SHADER_READ);
		reflection.SetSampler("combinedTex", Sampler::GetDefaultLinearSampler());

		// Output
		reflection.AddOutput("out");
		reflection.SetBindPoint("out", FResourceBindPoint::COLOR_ATTACHMENT);
		reflection.SetFormat("out", EFormat::B8G8R8A8_UNORM);
		reflection.SetTextureSize("out", 0, 0); // 0 is invalid - defaults to default (window size)

		reflection.AddOutput("depth");
		reflection.SetBindPoint("depth", FResourceBindPoint::DEPTH_STENCIL);
		reflection.SetFormat("depth", EFormat::DEPTH_STENCIL);

		PolyID vertShader = ShaderManager::CreateShader("shaders/pbr.vert", FShaderStage::VERTEX);
		PolyID fragShader = ShaderManager::CreateShader("shaders/pbr.frag", FShaderStage::FRAGMENT);

		SetShaderID(FShaderStage::VERTEX, vertShader);
		SetShaderID(FShaderStage::FRAGMENT, fragShader);

		return reflection;
	}

	void PBRPass::Execute(const RenderContext& context, const RenderData& renderData)
	{
		m_SceneRenderer.Execute(context);
	}
}
