#pragma once

#include <string>

namespace Poly
{
	// A TextureHandle field (see common/bindless.slang) in the push constants of a pass
	struct TextureField
	{
		std::string ShaderName;
		uint32      Offset = 0; // byte offset within the push constant block
	};
} // namespace Poly
