#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	struct ShaderDesc
	{
		std::string			EntryPoint	= "main";
		FShaderStage		ShaderStage	= FShaderStage::NONE;
		std::vector<byte>	ShaderCode;
	};

	class Shader
	{
	public:
		CLASS_ABSTRACT(Shader);

		/**
		 * Init the Buffer object
		 * @param desc	Shader creation description
		*/
		virtual void Init(const ShaderDesc* pDesc) = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;
	};
}