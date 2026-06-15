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

	PassDeclaration& PassDeclaration::MapResource(EFeaturePort port, std::string_view shaderResourceName)
	{
		m_ResourceMappings.emplace_back(port, std::string(shaderResourceName));
		return *this;
	}

	PassDeclaration& PassDeclaration::MapGlobal(std::string_view globalName, std::string_view shaderGlobalName)
	{
		m_GlobalMappings.emplace_back(std::string(globalName), std::string(shaderGlobalName));
		return *this;
	}

	PassDeclaration& PassDeclaration::ImportResource(std::string_view resourceName, std::string_view shaderResourceName)
	{
		m_ImportedResources.emplace_back(std::string(resourceName), std::string(shaderResourceName));
		return *this;
	}

	PassDeclaration& PassDeclaration::ExportResource(std::string_view resourceName, std::string_view shaderResourceName)
	{
		m_ExportedResources.emplace_back(std::string(resourceName), std::string(shaderResourceName));
		return *this;
	}
} // namespace Poly
