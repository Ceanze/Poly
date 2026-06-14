#include "PassDeclaration.h"

namespace Poly
{
	PassDeclaration::PassDeclaration()
	    : m_GraphicsPipelineDecl(*this)
	{}

	PassDeclaration& PassDeclaration::WithShader(std::string_view shaderPath, FShaderStage stage)
	{
		m_Shaders.emplace_back(std::string(shaderPath), stage);
		return *this;
	}

	PassDeclaration& PassDeclaration::WithSetupFn(std::function<void(SetupContext&)> setupFn)
	{
		m_SetupFn = std::move(setupFn);
		return *this;
	}

	PassDeclaration& PassDeclaration::WithExecuteFn(std::function<void(ExecuteContext&)> executeFn)
	{
		m_ExecuteFn = std::move(executeFn);
		return *this;
	}

	PassDeclarationGraphicsPipeline& PassDeclaration::WithGraphicsPipeline()
	{
		return m_GraphicsPipelineDecl;
	}
} // namespace Poly
