#include "polypch.h"
#include "ShaderReflector.h"

#include <spirv_reflect.h>

namespace Poly
{
	static FResourceBindPoint SpvToPoly(SpvReflectDescriptorType type)
	{
		switch (type)
		{
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:					return FResourceBindPoint::SAMPLER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:	return FResourceBindPoint::SAMPLER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:				return FResourceBindPoint::SAMPLED_IMAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:				return FResourceBindPoint::STORAGE_IMAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:		return FResourceBindPoint::UNIFORM_TEXEL;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:		return FResourceBindPoint::STORAGE_TEXEL;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:			return FResourceBindPoint::UNIFORM;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:			return FResourceBindPoint::STORAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:	return FResourceBindPoint::UNIFORM_DYNAMIC;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:	return FResourceBindPoint::STORAGE_DYNAMIC;
		case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:			return FResourceBindPoint::INPUT_ATTACHMENT;
		default:													return FResourceBindPoint::NONE;
		}
	}

	ShaderReflection ShaderReflector::Reflect(FShaderStage shaderStage, const std::vector<byte>& data)
	{
		return ShaderReflection();
	}
}
