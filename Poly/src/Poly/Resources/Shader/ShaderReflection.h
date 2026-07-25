#pragma once

namespace Poly
{
	struct ShaderInputOutput
	{
		std::string Name;
		uint32      Location;
	};

	struct ShaderBinding
	{
		std::string        Name;
		uint32             Set;
		uint32             Binding;
		FResourceBindPoint DescriptorType;
		uint32             Count; // array size
	};

	struct ShaderPushConstant
	{
		std::string Name;
		uint32      Size;
		uint32      Offset;
	};

	// Describes where the shared BINDLESS_PUSH_CONSTANTS macro's textureIndices[]/bufferAddresses[]
	// arrays sit within this shader's push-constant block, if present (see plans/bindless.md).
	// Reflection only confirms the fixed layout is there and reports the offsets/counts - it does not
	// know which render-graph resource maps to which array slot (that's RenderProgramBuilder's job).
	struct ShaderBindlessLayout
	{
		bool   HasTextureSlots    = false;
		uint32 TextureSlotsOffset = 0; // byte offset of textureIndices[0] within the push constant block
		uint32 TextureSlotCount   = 0; // BINDLESS_MAX_SLOTS, read off the array's reflected element count

		bool   HasBufferSlots    = false;
		uint32 BufferSlotsOffset = 0; // byte offset of bufferAddresses[0] within the push constant block
		uint32 BufferSlotCount   = 0;
	};

	struct ShaderReflection
	{
		std::vector<ShaderInputOutput>  Inputs;
		std::vector<ShaderInputOutput>  Outputs;
		std::vector<ShaderBinding>      Bindings;
		std::vector<ShaderPushConstant> PushConstants;
		ShaderBindlessLayout            BindlessLayout;
	};
} // namespace Poly