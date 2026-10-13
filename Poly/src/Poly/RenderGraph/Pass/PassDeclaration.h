#pragma once

#include "IPassDeclaration.h"
#include "Platform/API/GraphicsPipeline.h"

#include <optional>
#include <string>
#include <vector>

/*
Example:
RenderGraph.RegisterPass("imgui")
    .WithShader("assets/shaders/ui_bindless.slang", FShaderStage::VERTEX)
    .WithShader("assets/shaders/ui_bindless.slang", FShaderStage::FRAGMENT)
    .MapResource(EFeaturePort::Color, "out_Color")
    .WithVertexLayout(sizeof(ImDrawVert))
    .WithVertexAttribute(0, EFormat::R32G32_SFLOAT, offsetof(ImDrawVert, pos))
    .WithVertexAttribute(1, EFormat::R32G32_SFLOAT, offsetof(ImDrawVert, uv))
    .WithVertexAttribute(2, EFormat::R8G8B8A8_UNORM, offsetof(ImDrawVert, col))
    .WithCullMode(ECullMode::NONE)
    .WithBlend(EBlendMode::AlphaBlend)
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

		PassDeclaration& WithShader(std::string_view shaderPath, FShaderStage stage) override;
		PassDeclaration& WithSetupFn(std::function<void(SetupContext&)> setupFn) override;
		PassDeclaration& WithExecuteFn(std::function<void(ExecuteContext&)> executeFn) override;
		PassDeclaration& OnQueue(FQueueType queue) override;

		PassDeclaration& WithTopology(ETopology topology) override;
		PassDeclaration& WithPolygonMode(EPolygonMode mode) override;
		PassDeclaration& WithCullMode(ECullMode mode) override;
		PassDeclaration& WithFrontFaceClockwise(bool clockwise = true) override;
		PassDeclaration& WithDepthTest(bool enable) override;
		PassDeclaration& WithDepthWrite(bool enable) override;
		PassDeclaration& WithDepthCompareOp(ECompareOp op) override;
		PassDeclaration& WithDepthBias(float constantFactor, float slopeFactor, float clamp = 0.0f) override;
		PassDeclaration& WithBlend(EBlendMode mode) override;
		PassDeclaration& WithBlend(const ColorBlendAttachmentDesc& blend) override;
		PassDeclaration& WithVertexLayout(uint32 stride, EVertexInputRate inputRate = EVertexInputRate::VERTEX) override;
		PassDeclaration& WithVertexAttribute(uint32 location, EFormat format, uint32 offset) override;
		PassDeclaration& WithPipelineOverride(std::function<void(GraphicsPipelineDesc&)> overrideFn) override;

		PassDeclaration& MapResource(EFeaturePort resourceName, std::string_view shaderResourceName, ELoadOp loadOp = ELoadOp::NONE) override;
		PassDeclaration& MapGlobal(std::string_view globalName, std::string_view shaderGlobalName) override;
		PassDeclaration& ImportResource(std::string_view resourceName, std::string_view shaderResourceName) override;
		PassDeclaration& ExportResource(std::string_view resourceName, std::string_view shaderResourceName) override;
		PassDeclaration& ReadResource(std::string_view resourceName, FResourceState state) override;
		PassDeclaration& WriteResource(std::string_view resourceName, FResourceState state) override;

		std::string_view GetName() const { return m_Name; }
		FQueueType       GetQueue() const { return m_Queue; }

		const std::vector<std::pair<std::string, FShaderStage>>& GetShaders() const { return m_Shaders; }
		const std::function<void(ExecuteContext&)>&              GetExecuteFn() const { return m_ExecuteFn; }

		void CallSetupFn(SetupContext& ctx) const;
		void CallExecuteFn(ExecuteContext& ctx) const;

		const std::vector<ResourceMapping>&                     GetResourceMappings() const { return m_ResourceMappings; }
		const std::vector<std::pair<std::string, std::string>>& GetGlobalMappings() const { return m_GlobalMappings; }
		const std::vector<std::pair<std::string, std::string>>& GetImportedResources() const { return m_ImportedResources; }
		const std::vector<std::pair<std::string, std::string>>& GetExportedResources() const { return m_ExportedResources; }
		const std::vector<ResourceUse>&                         GetResourceUses() const { return m_ResourceUses; }

		/*
		 * Resolves the pipeline state of the pass: the defaults, what follows from the mapped feature ports (depth
		 * test/write, one blend attachment per color output) and whatever was set explicitly.
		 */
		GraphicsPipelineDesc BuildPipelineDesc() const;

	private:
		const std::string m_Name;
		FQueueType        m_Queue = FQueueType::GRAPHICS;

		std::vector<std::pair<std::string, FShaderStage>> m_Shaders;
		std::function<void(SetupContext&)>                m_SetupFn;
		std::function<void(ExecuteContext&)>              m_ExecuteFn;

		// Explicitly set pipeline state. Depth test/write, blend attachments and the vertex binding are resolved in BuildPipelineDesc()
		GraphicsPipelineDesc                       m_PipelineDesc;
		std::optional<bool>                        m_DepthTest;
		std::optional<bool>                        m_DepthWrite;
		ColorBlendAttachmentDesc                   m_Blend;
		uint32                                     m_VertexStride    = 0;
		EVertexInputRate                           m_VertexInputRate = EVertexInputRate::VERTEX;
		std::function<void(GraphicsPipelineDesc&)> m_PipelineOverrideFn;

		std::vector<ResourceMapping>                     m_ResourceMappings;
		std::vector<std::pair<std::string, std::string>> m_GlobalMappings;
		std::vector<std::pair<std::string, std::string>> m_ImportedResources;
		std::vector<std::pair<std::string, std::string>> m_ExportedResources;
		std::vector<ResourceUse>                         m_ResourceUses;
	};
} // namespace Poly
