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
            // Stride/VertexInputRate only need to be set once - the Vulkan backend reads them off
            // VertexInputs[0] as the one shared binding for the whole pipeline (see PVKGraphicsPipeline::Init).
            .Stride(sizeof(ImDrawVert))
            .VertexInputRate(EVertexInputRate::VERTEX)
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

	// A mapped feature port, plus an optional attachment load-op override. LoadOpOverride left at
	// ELoadOp::NONE means "auto" - RenderProgramBuilder infers CLEAR on a resource's first write in
	// the program and LOAD on subsequent writes (see PlanSynchronization).
	struct ResourceMapping
	{
		EFeaturePort Port;
		std::string  ShaderResourceName;
		ELoadOp      LoadOpOverride = ELoadOp::NONE;
	};

	// A resource used by a pass without a shader binding, see IPassDeclaration::ReadResource/WriteResource.
	struct ResourceUse
	{
		std::string    Name;
		FResourceState State;
		bool           IsWrite;
	};

	class PassDeclaration : public IPassDeclaration
	{
	public:
		PassDeclaration(std::string_view name);
		~PassDeclaration() = default;

		PassDeclaration&                 WithShader(std::string_view shaderPath, FShaderStage stage) override;
		PassDeclaration&                 WithSetupFn(std::function<void(SetupContext&)> setupFn) override;
		PassDeclaration&                 WithExecuteFn(std::function<void(ExecuteContext&)> executeFn) override;
		PassDeclaration&                 OnQueue(FQueueType queue) override;
		PassDeclarationGraphicsPipeline& WithGraphicsPipeline() override;

		PassDeclaration& MapResource(EFeaturePort resourceName, std::string_view shaderResourceName, ELoadOp loadOp = ELoadOp::NONE) override;
		PassDeclaration& MapGlobal(std::string_view globalName, std::string_view shaderGlobalName) override;
		PassDeclaration& ImportResource(std::string_view resourceName, std::string_view shaderResourceName) override;
		PassDeclaration& ExportResource(std::string_view resourceName, std::string_view shaderResourceName) override;
		PassDeclaration& ReadResource(std::string_view resourceName, FResourceState state) override;
		PassDeclaration& WriteResource(std::string_view resourceName, FResourceState state) override;

		std::string_view GetName() const { return m_Name; }
		FQueueType       GetQueue() const { return m_Queue; }

		const std::vector<std::pair<std::string, FShaderStage>>& GetShaders() const { return m_Shaders; }
		const PassDeclarationGraphicsPipeline&                   GetGraphicsPipeline() const { return m_GraphicsPipelineDecl; }
		const std::function<void(ExecuteContext&)>&              GetExecuteFn() const { return m_ExecuteFn; }

		void CallSetupFn(SetupContext& ctx) const;
		void CallExecuteFn(ExecuteContext& ctx) const;

		const std::vector<ResourceMapping>&                      GetResourceMappings() const { return m_ResourceMappings; }
		const std::vector<std::pair<std::string, std::string>>&  GetGlobalMappings() const { return m_GlobalMappings; }
		const std::vector<std::pair<std::string, std::string>>&  GetImportedResources() const { return m_ImportedResources; }
		const std::vector<std::pair<std::string, std::string>>&  GetExportedResources() const { return m_ExportedResources; }
		const std::vector<ResourceUse>&                          GetResourceUses() const { return m_ResourceUses; }

		// Idea: A pass declaration should always be able to provide a full declaration of a pass for creation
		// of graphic resources. Meaning, for instance, the pipeline declaration is always valid, no matter if the
		// user created a custom (correct) one, or not.

	private:
		const std::string m_Name;
		FQueueType        m_Queue = FQueueType::GRAPHICS;

		std::vector<std::pair<std::string, FShaderStage>> m_Shaders;
		std::function<void(SetupContext&)>                m_SetupFn;
		std::function<void(ExecuteContext&)>              m_ExecuteFn;
		PassDeclarationGraphicsPipeline                   m_GraphicsPipelineDecl;

		std::vector<ResourceMapping>                      m_ResourceMappings;
		std::vector<std::pair<std::string, std::string>>  m_GlobalMappings;
		std::vector<std::pair<std::string, std::string>>  m_ImportedResources;
		std::vector<std::pair<std::string, std::string>>  m_ExportedResources;
		std::vector<ResourceUse>                          m_ResourceUses;
	};
} // namespace Poly
