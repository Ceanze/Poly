#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Resources/Shader/ShaderReflection.h"

#include <map>

namespace Poly
{
	class Shader;

	class ShaderManager
	{
	public:
		ShaderManager() = default;

		static void Init();
		static void Release();

		static PolyID CreateShader(std::string_view path, FShaderStage shaderStage);
		static Ref<Shader> GetShader(PolyID shaderID);
		static const ShaderReflection& GetReflection(PolyID shaderID);

		// TODO: Add a "onShaderUpdated" callback/notifier

	private:
		struct ShaderData
		{
			Ref<Shader> pShader;
			ShaderReflection reflection;
		};

		static std::map<PolyID, ShaderData> s_Shaders;
	};
}