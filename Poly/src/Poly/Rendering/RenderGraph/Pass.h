#pragma once

#include "ResourceGUID.h"
#include "Reflection/PassReflection.h"
#include "RenderGraphTypes.h"

namespace Poly
{
	struct PipelineDesc;

	class Shader;
	class RenderData;
	class RenderContext;
	class RenderGraphProgram;

	class Pass
	{
	public:
		enum class Type
		{
			NONE,
			RENDER,
			COMPUTE,
			SYNC
		};

		struct ExternalResourceData
		{
			ResourceGUID SrcGUID;
			ResourceGUID DstGUID;
		};

	public:
		Pass() = default;
		virtual ~Pass() = default;

		/**
		 * Informs the RenderGraph about the inputs and outputs of the RenderPass
		 * Called once during re/compilation of the render graph
		 * @return A PassReflection created for this render pass
		 */
		virtual PassReflection Reflect() = 0;

		/**
		 * OPTIONAL
		 * Update the pass, called once per frame before Execute()
		 * Use this to update internal pass resources before Execute()
		 */
		virtual void Update(const RenderContext& context) {};

		/**
		 * Execute the Pass, called once per frame after Update()
		 */
		virtual void Execute(const RenderContext& context, const RenderData& renderData) = 0;

		/**
		 * OPTIONAL
		 * Compile or recompile the Pass
		 * Called once during re/compilation of the render graph
		 */
		virtual void Compile() {};

		/**
		 * @return name of pass
		 */
		std::string GetName() const { return p_Name; }

		/**
		 * @return read-only vector of external resources
		 */
		const std::vector<ExternalResourceData>& GetExternalResources() const { return p_ExternalResources; }

		/**
		 * @return type of pass
		 */
		Pass::Type GetPassType() const { return p_Type; }

		/**
		 * Sets a shader to be used for that shader type
		 * @param shaderStage - stage of shader
		 * @param pShader - shader
		 */
		void SetShaderID(FShaderStage shaderStage, PolyID shaderID) { p_ShaderStages[shaderStage] = shaderID; }

		/**
		 * @param shaderStage - stage of shader
		 * @return shader of desired shader type
		 */
		PolyID GetShaderID(FShaderStage shaderStage) const { return p_ShaderStages.at(shaderStage); }

		/**
		* @return true if instanced scene rendering is enabled - see `ToggleInstancedSceneRendering` for more info
		*/
		bool IsInstancedSceneRenderingEnabled() const { return m_IsInstancedSceneRenderingEnabled; }

		//--------------------------------------------
		// Custom graphics types for advanced usages |
		//--------------------------------------------

		/**
		 * @return true if the Pass uses any custom graphics types
		 */
		bool UsesCustomTypes() const { return p_pPipelineDesc != nullptr; }

		/**
		 * Get the custom pipeline desc, nullptr if it is not being used
		 *
		 * @return custom pipeline desc pointer
		 */
		PipelineDesc* GetCustomPipelineDesc() const { return p_pPipelineDesc.get(); }

	protected:
		/**
		 * Sets the custom pipeline desc to be used instead of the generated pipeline desc
		 *
		 * @param pPipeline
		 */
		void SetCustomPipelineDesc(Ref<PipelineDesc> pPipelineDesc) { p_pPipelineDesc = pPipelineDesc; }

		/**
		* Enables or disables the use of instanced rendering based on the scene structure.
		* If enabled, then the `Execute()` call will be called once per instance.
		* The Instance Index will be provided in the `RenderContext`
		* If disabled, then `Execute()` is only called once per pass
		* 
		* @param enable - enable or disable the feature
		*/
		void ToggleInstancedSceneRendering(bool enable) { m_IsInstancedSceneRenderingEnabled = enable; }

	protected:
		friend class RenderGraph;
		std::string	p_Name	= "";
		Pass::Type	p_Type	= Pass::Type::NONE;
		std::unordered_map<FShaderStage, PolyID> p_ShaderStages;
		bool m_IsInstancedSceneRenderingEnabled = false;

		// Pair structure: first: External resource name (src), second: Render pass input name (dst)
		std::vector<ExternalResourceData> p_ExternalResources;

		// Variables for custom usage of passes
		Ref<PipelineDesc> p_pPipelineDesc = nullptr;
	};
}