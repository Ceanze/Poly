#pragma once

#include "IPassDeclaration.h"
#include "PassDeclarationGraphicsPipeline.h"

/*
Example:
RenderGraph.RegisterPass("imgui")
    .WithShader("shaders/imgui/ui.vert", FShaderStage::VERTEX)
    .WithShader("shaders/imgui/ui.frag", FShaderStage::FRAGMENT)
    .WithGraphicsPipeline()
        .AddVertexInput()
            .Binding(0)
            .Location(0)
            .Format(EFormat::R32G32_SFLOAT)
            .Offset(offsetof(ImDrawVert, pos))
        .AddVertexInput()
            .Binding(0)
            .Location(1)
            .Format(EFormat::R32G32_SFLOAT)
            .Offset(offsetof(ImDrawVert, uv))
        .AddVertexInput()
            .Binding(0)
            .Location(2)
            .Format(EFormat::R8G8B8A8_UNORM)
            .Offset(offsetof(ImDrawVert, col))
        .FinishVertexInput()
        .FinishPipeline()
    .WithExecuteFn( ... );
*/

namespace Poly
{
	class SetupContext;
	class ExecuteContext;

	class PassDeclaration : public IPassDeclaration
	{
	public:
		PassDeclaration();
		~PassDeclaration() = default;

		PassDeclaration&                 WithShader(std::string_view shaderPath, FShaderStage stage) override;
		PassDeclaration&                 WithSetupFn(std::function<void(SetupContext&)> setupFn) override;
		PassDeclaration&                 WithExecuteFn(std::function<void(ExecuteContext&)> executeFn) override;
		PassDeclarationGraphicsPipeline& WithGraphicsPipeline() override;

		// Idea: A pass declaration should always be able to provide a full declaration of a pass for creation
		// of graphic resources. Meaning, for instance, the pipeline declaration is always valid, no matter if the
		// user created a custom (correct) one, or not.

	private:
		std::vector<std::pair<std::string, FShaderStage>> m_Shaders;
		std::function<void(SetupContext&)>                m_SetupFn;
		std::function<void(ExecuteContext&)>              m_ExecuteFn;
		PassDeclarationGraphicsPipeline                   m_GraphicsPipelineDecl;
	};
} // namespace Poly
