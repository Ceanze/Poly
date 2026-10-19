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
    .WithVertexLayout("ImDrawVert")
    .WithCullMode(ECullMode::NONE)
    .WithBlend(EBlendMode::AlphaBlend)
    .WithExecutor("ImGui");
*/

namespace Poly
{
	class ExecuteContext;
	struct VertexLayout;

	class PassDeclaration : public IPassDeclaration
	{
	public:
		PassDeclaration(std::string_view name);
		~PassDeclaration() = default;

		PassDeclaration& WithShader(std::string_view shaderPath, FShaderStage stage) override;
		PassDeclaration& WithExecuteFn(std::function<void(ExecuteContext&)> executeFn) override;
		PassDeclaration& WithExecutor(std::string_view executorName, std::string_view paramsJson = "") override;
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
		PassDeclaration& WithStencil(const StencilOpStateDesc& state) override;
		PassDeclaration& WithStencil(const StencilOpStateDesc& front, const StencilOpStateDesc& back) override;
		PassDeclaration& WithLogicOp(ELogicOp op) override;
		PassDeclaration& WithVertexLayout(std::string_view layoutName) override;

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

		/*
		 * Replaces everything about the pass with a description, e.g. one read from a pass file. The name of the pass and
		 * of the description must match. Removes the execute function of the pass.
		 */
		void SetDesc(PassDesc desc);

		const std::vector<PassShader>&              GetShaders() const { return m_Desc.Shaders; }
		const std::function<void(ExecuteContext&)>& GetExecuteFn() const { return m_ExecuteFn; }


		const std::vector<ResourceMapping>& GetResourceMappings() const { return m_Desc.ResourceMappings; }
		const std::vector<ResourceBinding>& GetGlobalMappings() const { return m_Desc.GlobalMappings; }
		const std::vector<ResourceBinding>& GetImportedResources() const { return m_Desc.ImportedResources; }
		const std::vector<ResourceBinding>& GetExportedResources() const { return m_Desc.ExportedResources; }
		const std::vector<ResourceUse>&     GetResourceUses() const { return m_Desc.ResourceUses; }

		/*
		 * Resolves the pipeline state of the pass: the defaults, what follows from the mapped feature ports (depth
		 * test/write, one blend attachment per color output) and whatever was set explicitly.
		 * @param pVertexLayout The registered vertex layout of the pass, nullptr if the pass binds no vertex buffer.
		 */
		GraphicsPipelineDesc BuildPipelineDesc(const VertexLayout* pVertexLayout) const;

	private:
		PassDesc m_Desc;

		std::function<void(ExecuteContext&)> m_ExecuteFn;
	};
} // namespace Poly
