#pragma once

#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <functional>
#include <string_view>

namespace Poly
{
	class SetupContext;
	class ExecuteContext;
	class PassDeclaration;
	class IPassDeclarationGraphicsPipeline;

	class IPassDeclaration
	{
	public:
		virtual ~IPassDeclaration() = default;

		/*
		 * Adds a shader to the pass. Multiple shaders can be added, but only one per stage.
		 */
		virtual IPassDeclaration& WithShader(std::string_view shaderPath, FShaderStage stage) = 0;

		/*
		 * Add a setup function executed when the render program is compiled.
		 */
		virtual IPassDeclaration& WithSetupFn(std::function<void(SetupContext&)> setupFn) = 0;

		/*
		 * Declares which queue this pass should execute on. Defaults to FQueueType::GRAPHICS if never called.
		 *
		 * @param queue The queue to run this pass on.
		 * @return A reference to this for chaining.
		 */
		virtual IPassDeclaration& OnQueue(FQueueType queue) = 0;

		/*
		 * Add an execute function called every frame during render program execution.
		 */
		virtual IPassDeclaration& WithExecuteFn(std::function<void(ExecuteContext&)> executeFn) = 0;

		/*
		 * Starts a pipeline override declaration. Finish with .FinishPipeline() to return here.
		 */
		virtual IPassDeclarationGraphicsPipeline& WithGraphicsPipeline() = 0;

		/*
		 * Maps a feature port to a shader resource of the pass. This works as a connection between the stricter feature pipeline
		 * and the more flexible pass pipeline. Mapping a feature port means that it is then exposed to other features in the pipeline,
		 * as a read if the port is an input, or a write if it is an output.
		 *
		 * @param port The feature port to map.
		 * @param shaderResourceName The name of the shader resource to map to. This should match a resource in the shader of the pass.
		 * @param loadOp Overrides the auto-inferred attachment load op (first write in the program clears, later writes load).
		 *               Leave at ELoadOp::NONE to keep the automatic behavior.
		 * @return A reference to this for chaining.
		 */
		virtual PassDeclaration& MapResource(EFeaturePort port, std::string_view shaderResourceName, ELoadOp loadOp = ELoadOp::NONE) = 0;

		/*
		 * Maps a global variable to a shader global of the pass.
		 *
		 * @param globalName The name of the global variable to map.
		 * @param shaderGlobalName The name of the shader global to map to. This should match a global in the shader of the pass.
		 * @return A reference to this for chaining.
		 */
		virtual PassDeclaration& MapGlobal(std::string_view globalName, std::string_view shaderGlobalName) = 0;

		/*
		 * Imports a resource to the pass. This is different to mapping feature ports, as it isn't limited to the available feature ports.
		 * Importing a resource means that the pass can have a dependency to a resource from another pass, without the need of a feature port.
		 * For the importing to be valid at compilation, another feature must export the same resource name before this pass.
		 * @param resourceName The name of the resource to import. This should match the exported resource name of another pass.
		 * @param shaderResourceName The name of the shader resource to map to. This should match a resource in the shader of the pass.
		 * @return A reference to this for chaining
		 */
		virtual PassDeclaration& ImportResource(std::string_view resourceName, std::string_view shaderResourceName) = 0;

		/*
		 * Exports a resource from the pass. This is different to mapping feature ports, as it isn't limited to the available feature ports.
		 * Exporting a resource means that the pass can have a dependency to a resource from another pass, without the need of a feature port.
		 * For the exporting to be valid at compilation, another pass must import the same resource name after this pass.
		 * @param resourceName The name of the resource to export. This should match the imported resource name of another pass.
		 * @param shaderResourceName The name of the shader resource to map to. This should match a resource in the shader of the pass.
		 * @return A reference to this for chaining
		 */
		virtual PassDeclaration& ExportResource(std::string_view resourceName, std::string_view shaderResourceName) = 0;
	};
} // namespace Poly
