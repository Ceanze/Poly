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

	enum class EBindlessFieldType
	{
		Buffer, // a pointer, filled in with the device address of a buffer (8 bytes)
		Texture // a TextureHandle (see common/bindless.slang), filled in with a packed texture/sampler heap index (4 bytes)
	};

	// A field of the shader's push-constant block that refers to a resource. Reflection only reports where
	// the field sits - which render-graph resource it gets is decided by name
	struct ShaderBindlessField
	{
		std::string        Name;
		uint32             Offset; // byte offset within the push constant block
		EBindlessFieldType Type;
	};

	struct ShaderReflection
	{
		std::vector<ShaderInputOutput>   Inputs;
		std::vector<ShaderInputOutput>   Outputs;
		std::vector<ShaderBinding>       Bindings;
		std::vector<ShaderPushConstant>  PushConstants;
		std::vector<ShaderBindlessField> BindlessFields;
	};
} // namespace Poly