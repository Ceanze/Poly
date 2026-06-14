#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <functional>
#include <string_view>

namespace Poly
{
	class SetupContext;
	class ExecuteContext;
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
		 * Add an execute function called every frame during render program execution.
		 */
		virtual IPassDeclaration& WithExecuteFn(std::function<void(ExecuteContext&)> executeFn) = 0;

		/*
		 * Starts a pipeline override declaration. Finish with .FinishPipeline() to return here.
		 */
		virtual IPassDeclarationGraphicsPipeline& WithGraphicsPipeline() = 0;
	};
} // namespace Poly
