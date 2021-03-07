#pragma once

namespace Poly
{
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
		 * Execute the RenderPass
		 */
		virtual void Execute(/* Render Context, Render Data */) = 0;

		/**
		 * OPTIONAL
		 * Compile or recompile the RenderPass
		 */
		virtual void Compile() {};

		/**
		 * @return name of render pass
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

	protected:
		friend class RenderGraph;
		std::string	p_Name	= "";
		Pass::Type	p_Type	= Pass::Type::NONE;

		// Pair structure: first: External resource name (src), second: Render pass input name (dst)
		std::vector<std::pair<std::string, std::string>> p_ExternalResources;
	};
}