#pragma once

#include "RenderPassReflection.h"
#include "Pass.h"

namespace Poly
{
	/**
	 * Reflect is creating the inputs and outputs of the RenderPass
	 * Execute is called each time the RenderPass should execute, commonly each frame
	 * Compile is called on first-time compilation and recompilation
	 */

	class RenderPass : public Pass
	{
	public:
		RenderPass() { p_Type = Pass::Type::RENDER; }
		virtual ~RenderPass() = default;

		/**
		 * Informs the RenderGraph about the inputs and outputs of the RenderPass
		 * @return A RenderPassReflection created for this render pass
		 */
		virtual RenderPassReflection Reflect() = 0;

		/**
		 * Execute the RenderPass
		 */
		virtual void Execute(const RenderContext& context, const RenderData& renderData) = 0;

		/**
		 * OPTIONAL
		 * Compile or recompile the RenderPass
		 */
		virtual void Compile() {};

	protected:
		friend class RenderGraph;

		// TODO: If/when scenes are added make it a member variable of the renderpass (Ref<Scene>)
	};
}