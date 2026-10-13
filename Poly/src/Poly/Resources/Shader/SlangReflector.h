#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Resources/Shader/ShaderReflection.h"

namespace slang
{
	struct ShaderReflection;
}

namespace Poly
{
	class SlangReflector
	{
	public:
		CLASS_STATIC(SlangReflector);

		/**
		 * Reflects a linked slang program with a single entry point
		 * @param pLayout - layout of the linked program (slang::ProgramLayout)
		 * @return reflection of the entry point's stage, empty if pLayout is null
		 */
		static ShaderReflection Reflect(slang::ShaderReflection* pLayout);
	};
} // namespace Poly
