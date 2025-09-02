#include "polypch.h"
#include "TestPass.h"
#include "../RenderContext.h"
#include "../RenderData.h"
#include "Platform/API/Sampler.h"
#include "Poly/Resources/ResourceLoader.h"
#include "Poly/Resources/Shader/ShaderManager.h"

namespace Poly
{
	PassReflection TestPass::Reflect()
	{
		PassReflection reflection;

		PolyID vertShader = ShaderManager::CreateShader("shaders/vert.glsl", FShaderStage::VERTEX);
		PolyID fragShader = ShaderManager::CreateShader("shaders/frag.glsl", FShaderStage::FRAGMENT);

		SetShaderID(FShaderStage::VERTEX, vertShader);
		SetShaderID(FShaderStage::FRAGMENT, fragShader);

		reflection.AddShader(vertShader);
		reflection.AddShader(fragShader);

		// Set texture formats and samplers, all are on set 2 for TestPass
		for (PassField* output : reflection.GetFields(FFieldVisibility::INPUT))
		{
			if (output->GetSet() == 2)
			{
				output->Format(EFormat::R8G8B8A8_UNORM)
					.SetSampler(Sampler::GetDefaultLinearSampler());
			}
		}

		reflection.GetField("outColor")
			.Format(EFormat::B8G8R8A8_UNORM)
			.BindPoint(FResourceBindPoint::COLOR_ATTACHMENT);

		reflection.AddOutput("depth")
			.BindPoint(FResourceBindPoint::DEPTH_STENCIL)
			.Format(EFormat::DEPTH_STENCIL);

		return reflection;
	}

	void TestPass::Execute(const RenderContext& context, const RenderData& renderData)
	{
		m_SceneRenderer.Execute(context);
	}
}
