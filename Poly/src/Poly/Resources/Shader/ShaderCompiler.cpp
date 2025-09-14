#include "polypch.h"
#include "ShaderCompiler.h"

#include "Poly/Resources/GLSLang.h"
#include "Poly/Resources/IOManager.h"

#include <fstream>

namespace Poly
{
	const std::vector<byte> ShaderCompiler::CompileGLSL(const std::string& filename, const std::string& folder, FShaderStage shaderStage)
	{
		if (IOManager::GetFileExtension(filename) == "spv")
		{ 
			std::ifstream file(folder + filename, std::ios_base::binary);

			std::string spvFile((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			std::vector<byte> correctType = std::vector<byte>(reinterpret_cast<byte*>(spvFile.data()), reinterpret_cast<byte*>(spvFile.data()) + spvFile.length());
			return correctType;
		}

		EShLanguage shaderType = ConvertShaderStageGLSLang(shaderStage);

		std::ifstream file(folder + filename);

		// Load and transfer content to string
		POLY_VALIDATE(file.is_open(), "Failed to open shader file: {0} \n at path: {1}", filename, folder);

		const std::string inputGLSL((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		const char* pInputCString = inputGLSL.c_str();

		// Setup glslang shader
		glslang::TShader shader(shaderType);
		shader.setDebugInfo(true);

		shader.setStrings(&pInputCString, 1);

		// Setup resources (might save values elsewhere or as constants)
		int clientInputSemanticsVersion = 100;
		glslang::EShTargetClientVersion vulkanClientVersion	= glslang::EShTargetVulkan_1_3; // VULKAN 1.2 (latest)
		glslang::EShTargetLanguageVersion targetVersion		= glslang::EShTargetSpv_1_5; // SPV 1.5 (latest)
		const TBuiltInResource* pResources					= GetDefaultBuiltInResources();
		EShMessages messages								= static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault);
		const int defaultGLSLVersion						= 450; // Shader version 450 (latest)

		shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, clientInputSemanticsVersion);
		shader.setEnvClient(glslang::EShClientVulkan, vulkanClientVersion);
		shader.setEnvTarget(glslang::EShTargetSpv, targetVersion);

		DirStackFileIncluder includer;
		includer.pushExternalLocalDirectory(folder);

		std::string preprocessedGLSL;

		if (!shader.preprocess(pResources, defaultGLSLVersion, ENoProfile, false, false, messages, &preprocessedGLSL, includer))
			POLY_CORE_WARN("GLSL preprocessing failed for: {0} \n {1} \n {2}", filename, shader.getInfoLog(), shader.getInfoDebugLog());

		const char* pPreprocessedCString = preprocessedGLSL.c_str();
		shader.setStrings(&pPreprocessedCString, 1);

		if (!shader.parse(pResources, defaultGLSLVersion, false, messages))
			POLY_CORE_WARN("GLSL parsing failed for: {0} \n {1} \n {2}", filename, shader.getInfoLog(), shader.getInfoDebugLog());

		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(messages))
			POLY_CORE_WARN("GLSL linking failed for: {0} \n {1} \n {2}", filename, shader.getInfoLog(), shader.getInfoDebugLog());

		std::vector<uint32_t> sprirv;
		spv::SpvBuildLogger logger;
		glslang::SpvOptions spvOptions;
		spvOptions.generateDebugInfo = true;
		glslang::GlslangToSpv(*program.getIntermediate(shaderType), sprirv, &logger, &spvOptions);

		const uint32_t sourceSize = static_cast<uint32_t>(sprirv.size()) * sizeof(uint32_t);
		std::vector<byte> correctType = std::vector<byte>(reinterpret_cast<byte*>(sprirv.data()), reinterpret_cast<byte*>(sprirv.data()) + sourceSize);

		// TODO: Return shader or other object instead?
		return correctType;
	}
}