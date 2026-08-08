#include "ShaderCompiler.h"

#include "Poly/Resources/GLSLang.h"
#include "Poly/Resources/PathUtils.h"
#include "Poly/Resources/VFS/VirtualFileSystem.h"
#include "polypch.h"

#include <fstream>

namespace Poly
{
	const std::vector<byte> ShaderCompiler::CompileGLSL(std::string_view path, FShaderStage shaderStage)
	{
		if (PathUtils::GetExtension(path) == "spv")
		{
			return VirtualFileSystem::Read(path);
		}

		EShLanguage shaderType = ConvertShaderStageGLSLang(shaderStage);

		const std::string inputGLSL     = VirtualFileSystem::ReadText(path);
		const char*       pInputCString = inputGLSL.c_str();

		// Setup glslang shader
		glslang::TShader shader(shaderType);

		shader.setStrings(&pInputCString, 1);

		// Setup resources (might save values elsewhere or as constants)
		int                               clientInputSemanticsVersion = 100;
		glslang::EShTargetClientVersion   vulkanClientVersion         = glslang::EShTargetVulkan_1_3; // VULKAN 1.2 (latest)
		glslang::EShTargetLanguageVersion targetVersion               = glslang::EShTargetSpv_1_5;    // SPV 1.5 (latest)
		const TBuiltInResource*           pResources                  = GetDefaultBuiltInResources();
		EShMessages                       messages                    = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault);
		const int                         defaultGLSLVersion          = 450; // Shader version 450 (latest)

		shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, clientInputSemanticsVersion);
		shader.setEnvClient(glslang::EShClientVulkan, vulkanClientVersion);
		shader.setEnvTarget(glslang::EShTargetSpv, targetVersion);

		DirStackFileIncluder includer;
		includer.pushExternalDirectory(PathUtils::GetDirectoryPath(VirtualFileSystem::Resolve(path)));

		std::string preprocessedGLSL;

		if (!shader.preprocess(pResources, defaultGLSLVersion, ENoProfile, false, false, messages, &preprocessedGLSL, includer))
			POLY_CORE_WARN("GLSL preprocessing failed for: {0} \n {1} \n {2}", path, shader.getInfoLog(), shader.getInfoDebugLog());

		const char* pPreprocessedCString = preprocessedGLSL.c_str();
		shader.setStrings(&pPreprocessedCString, 1);

		if (!shader.parse(pResources, defaultGLSLVersion, false, messages))
			POLY_CORE_WARN("GLSL parsing failed for: {0} \n {1} \n {2}", path, shader.getInfoLog(), shader.getInfoDebugLog());

		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(messages))
			POLY_CORE_WARN("GLSL linking failed for: {0} \n {1} \n {2}", path, shader.getInfoLog(), shader.getInfoDebugLog());

		std::vector<uint32_t> sprirv;
		spv::SpvBuildLogger   logger;
		glslang::SpvOptions   spvOptions;
		glslang::GlslangToSpv(*program.getIntermediate(shaderType), sprirv, &logger, &spvOptions);

		const uint32_t    sourceSize  = static_cast<uint32_t>(sprirv.size()) * sizeof(uint32_t);
		std::vector<byte> correctType = std::vector<byte>(reinterpret_cast<byte*>(sprirv.data()), reinterpret_cast<byte*>(sprirv.data()) + sourceSize);

		// TODO: Return shader or other object instead?
		return correctType;
	}
} // namespace Poly