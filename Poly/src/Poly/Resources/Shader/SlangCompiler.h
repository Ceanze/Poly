#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Resources/Shader/CompiledShader.h"

#include <unordered_map>

namespace slang
{
	struct IGlobalSession;
	struct ISession;
	struct IModule;
} // namespace slang

namespace Poly
{
	class SlangFileSystem;

	// Not thread safe, callers must serialize access
	class SlangCompiler
	{
	public:
		CLASS_REMOVE_COPY(SlangCompiler);
		CLASS_REMOVE_MOVE(SlangCompiler);

		SlangCompiler();
		~SlangCompiler();

		/**
		 * Compiles the entry point of the given stage in a .slang file to SPIR-V.
		 * The module (and everything it imports) is loaded once and reused for the other stages in the file.
		 * @param path - VFS path of the .slang file
		 * @param shaderStage - stage of the entry point to compile, the module must define exactly one for it
		 * @return SPIR-V code with the entry point named "main" and its reflection, the code is empty on failure
		 */
		CompiledShader Compile(std::string_view path, FShaderStage shaderStage);

	private:
		slang::IModule* LoadModule(std::string_view path);

		slang::IGlobalSession*  m_pGlobalSession = nullptr;
		slang::ISession*        m_pSession       = nullptr;
		Unique<SlangFileSystem> m_pFileSystem;

		// Keyed by VFS path
		std::unordered_map<std::string, slang::IModule*> m_Modules;
	};
} // namespace Poly
