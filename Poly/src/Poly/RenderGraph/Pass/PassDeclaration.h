#pragma once

#include "IPassDeclaration.h"
#include "PassDesc.h"
#include "Platform/API/GraphicsPipeline.h"

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

		std::string_view GetName() const { return m_Desc.Name; }
		FQueueType       GetQueue() const { return m_Desc.Queue; }

		/*
		 * Gets the data of the pass, everything but its functions.
		 */
		const PassDesc& GetDesc() const { return m_Desc; }

		const std::vector<PassShader>&              GetShaders() const { return m_Desc.Shaders; }
		const std::function<void(ExecuteContext&)>& GetExecuteFn() const { return m_ExecuteFn; }

		void CallSetupFn(SetupContext& ctx) const;
		void CallExecuteFn(ExecuteContext& ctx) const;

		const std::vector<ResourceMapping>& GetResourceMappings() const { return m_Desc.ResourceMappings; }
		const std::vector<ResourceBinding>& GetGlobalMappings() const { return m_Desc.GlobalMappings; }
		const std::vector<ResourceBinding>& GetImportedResources() const { return m_Desc.ImportedResources; }
		const std::vector<ResourceBinding>& GetExportedResources() const { return m_Desc.ExportedResources; }
		const std::vector<ResourceUse>&     GetResourceUses() const { return m_Desc.ResourceUses; }

		/*
		 * Resolves the pipeline state of the pass: the defaults, what follows from the mapped feature ports (depth
		 * test/write, one blend attachment per color output) and whatever was set explicitly.
		 */
		GraphicsPipelineDesc BuildPipelineDesc() const;

	private:
		PassDesc m_Desc;

		std::function<void(SetupContext&)>         m_SetupFn;
		std::function<void(ExecuteContext&)>       m_ExecuteFn;
		std::function<void(GraphicsPipelineDesc&)> m_PipelineOverrideFn;
	};
} // namespace Poly
