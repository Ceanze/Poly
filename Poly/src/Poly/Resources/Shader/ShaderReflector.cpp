#include "polypch.h"
#include "ShaderReflector.h"

#include <spirv_reflect.h>

namespace Poly
{
	//static FResourceBindPoint SpvToPoly(SpvReflectDescriptorType type)
	//{
	//	switch (type)
	//	{
	//	case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:					return FResourceBindPoint::SAMPLER;
	//	case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:	return FResourceBindPoint::SAMPLER;
	//	case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:				return FResourceBindPoint::SHADER_READ;
	//	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:				return FResourceBindPoint::STORAGE;
	//	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:		return FResourceBindPoint::UNIFORM;
	//	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:		return FResourceBindPoint::STORAGE;
	//	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:			return FResourceBindPoint::UNIFORM;
	//	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:			return FResourceBindPoint::STORAGE;
	//	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:	return FResourceBindPoint::UNIFORM;
	//	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:	return FResourceBindPoint::STORAGE;
	//	case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:			return FResourceBindPoint::INPUT_ATTACHMENT;
	//	default:												return FResourceBindPoint::NONE;
	//	}
	//}

	ShaderReflector::ShaderReflector()
	{
	}

	ShaderReflector& ShaderReflector::AddReflect(FShaderStage shaderStage, const std::string& path)
	{
		m_ReflectionStages.push_back({ shaderStage, path });
		return *this;
	}

	ShaderReflector& ShaderReflector::AddReflect(FShaderStage shaderStage, const std::vector<char>& data)
	{
		return *this;
	}

	ShaderReflection ShaderReflector::GenerateReflection()
	{
		return ShaderReflection();
	}
}
