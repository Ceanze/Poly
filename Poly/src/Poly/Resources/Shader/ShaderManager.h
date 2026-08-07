#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Resources/Shader/ShaderReflection.h"

#include <map>
#include <mutex>

namespace Poly
{
	class Shader;

	struct ShaderData
	{
		FShaderStage     ShaderStage = FShaderStage::NONE;
		Ref<Shader>      pShader;
		ShaderReflection Reflection;
	};

	class ShaderManager
	{
	public:
		ShaderManager() = default;

		static void Init();
		static void Release();

		static PolyID CreateShader(std::string_view path, FShaderStage shaderStage);

		static bool              ShaderExists(PolyID shaderID);
		static const ShaderData& GetShader(PolyID shaderID);

		// TODO: Add a "onShaderUpdated" callback/notifier

	private:
		// Guards s_Shaders - CreateShader()/GetShader() can now be called concurrently, e.g. from
		// RenderProgramInstance's per-pass lazy pipeline creation during multithreaded recording.
		static std::mutex                   s_Mutex;
		static std::map<PolyID, ShaderData> s_Shaders;
	};
} // namespace Poly