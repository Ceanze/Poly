#include "polypch.h"
#include "PBRPass.h"
#include "../RenderData.h"
#include "../RenderContext.h"
#include "Poly/Resources/ResourceLoader.h"
#include "Poly/Resources/Shader/ShaderManager.h"

#include "Poly/Rendering/RenderGraph/Reflection/PassReflection.h"

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

		PolyID vertShader = ShaderManager::CreateShader("shaders/pbr.vert", FShaderStage::VERTEX);
		PolyID fragShader = ShaderManager::CreateShader("shaders/pbr.frag", FShaderStage::FRAGMENT);

		SetShaderID(FShaderStage::VERTEX, vertShader);
		SetShaderID(FShaderStage::FRAGMENT, fragShader);

		reflection.AddShader(vertShader);
		reflection.AddShader(fragShader);

		// Set texture formats and samplers, all are on set 3 for PBRPass
		for (PassField* output : reflection.GetFields(FFieldVisibility::INPUT))
		{
			if (output->GetSet() == 3)
			{
				output->Format(EFormat::R8G8B8A8_UNORM);
			}
		}

		reflection.GetField("out_Color")
			.Format(EFormat::B8G8R8A8_UNORM)
			.BindPoint(FResourceBindPoint::COLOR_ATTACHMENT);

		reflection.AddOutput("depth")
			.BindPoint(FResourceBindPoint::DEPTH_STENCIL)
			.Format(EFormat::DEPTH_STENCIL);

		return reflection;
	}

	void PBRPass::Execute(const RenderContext& context, const RenderData& renderData)
	{
		m_SceneRenderer.Execute(context);
	}
}
