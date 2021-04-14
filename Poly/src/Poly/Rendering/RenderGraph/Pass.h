#pragma once

#include "PassReflection.h"
#include "RenderGraphTypes.h"

namespace Poly
{
	class Shader;
	class RenderContext;
	class RenderData;

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

	public:
		Pass() = default;
		virtual ~Pass() = default;

		/**
		 * Informs the RenderGraph about the inputs and outputs of the RenderPass
		 * @return A PassReflection created for this render pass
		 */
		virtual PassReflection Reflect() = 0;

		/**
		 * Execute the Pass
		 */
		virtual void Execute(const RenderContext& context, const RenderData& renderData) = 0;

		/**
		 * OPTIONAL
		 * Compile or recompile the Pass
		 */
		virtual void Compile() {};

		/**
		 * @return name of pass
		 */
		std::string GetName() const { return p_Name; }

		/**
		 * @return read-only vector of external resources
		 */
		const std::vector<std::pair<std::string, std::string>>& GetExternalResources() const { return p_ExternalResources; }

		/**
		 * @return type of pass
		 */
		Pass::Type GetPassType() const { return p_Type; }

		/**
		 * Sets a shader to be used for that shader type
		 * @param shaderStage - stage of shader
		 * @param pShader - shader
		 */
		void SetShader(FShaderStage shaderStage, Ref<Shader> pShader) { p_ShaderStages[shaderStage] = pShader; }

		/**
		 * @param shaderStage - stage of shader
		 * @return shader of desired shader type
		 */
		Ref<Shader> GetShader(FShaderStage shaderStage) const { return p_ShaderStages.at(shaderStage); }

	protected:
		friend class RenderGraph;
		std::string	p_Name	= "";
		Pass::Type	p_Type	= Pass::Type::NONE;
		std::unordered_map<FShaderStage, Ref<Shader>> p_ShaderStages;

		// Pair structure: first: External resource name (src), second: Render pass input name (dst)
		std::vector<std::pair<std::string, std::string>> p_ExternalResources;
	};
}