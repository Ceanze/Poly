#include "polypch.h"
#include "ShaderReflector.h"

#include <spirv_reflect.h>

namespace
{
	Poly::FResourceBindPoint SpvDescriptorTypeToPoly(SpvReflectDescriptorType type)
	{
		switch (type)
		{
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:					return Poly::FResourceBindPoint::SAMPLER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:	return Poly::FResourceBindPoint::SAMPLER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:				return Poly::FResourceBindPoint::SAMPLED_IMAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:				return Poly::FResourceBindPoint::STORAGE_IMAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:		return Poly::FResourceBindPoint::UNIFORM_TEXEL;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:		return Poly::FResourceBindPoint::STORAGE_TEXEL;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:			return Poly::FResourceBindPoint::UNIFORM;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:			return Poly::FResourceBindPoint::STORAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:	return Poly::FResourceBindPoint::UNIFORM_DYNAMIC;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:	return Poly::FResourceBindPoint::STORAGE_DYNAMIC;
		case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:			return Poly::FResourceBindPoint::INPUT_ATTACHMENT;
		default:													return Poly::FResourceBindPoint::NONE;
		}
	}
}

namespace Poly
{
	ShaderReflector::ShaderReflector(const std::vector<byte>& data)
	{
        m_Module = CreateUnique<SpvReflectShaderModule>();
        SpvReflectResult result = spvReflectCreateShaderModule(data.size() * sizeof(byte), data.data(), m_Module.get());
        m_isValid = result == SPV_REFLECT_RESULT_SUCCESS;
        POLY_VALIDATE(m_isValid, "Reflector module could not be created - result was {}", result);
	}

    ShaderReflector::~ShaderReflector()
    {
        spvReflectDestroyShaderModule(m_Module.get());
    }

	ShaderReflection ShaderReflector::Reflect()
	{
        ShaderReflection result;

        if (!m_isValid)
        {
            POLY_CORE_ERROR("Reflection cannot be created, shader module is invalid");
            return result;
        }

        SpvReflectShaderModule* module = m_Module.get();

        // Inputs (stage inputs e.g. vertex attributes)
        {
            uint32_t count = 0;
            spvReflectEnumerateInputVariables(module, &count, nullptr);
            std::vector<SpvReflectInterfaceVariable*> inputs(count);
            spvReflectEnumerateInputVariables(module, &count, inputs.data());

            for (auto* input : inputs)
            {
                // skip built-ins like gl_Position
                if (input->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
                    continue;

                result.Inputs.push_back({
                    input->name ? input->name : "",
                    input->location
                    });
            }
        }

        // Descriptor bindings
        {
            uint32_t count = 0;
            spvReflectEnumerateDescriptorBindings(module, &count, nullptr);
            std::vector<SpvReflectDescriptorBinding*> bindings(count);
            spvReflectEnumerateDescriptorBindings(module, &count, bindings.data());

            for (auto* binding : bindings)
            {
                result.Bindings.push_back({
                    binding->name ? binding->name : "",
                    binding->set,
                    binding->binding,
                    SpvDescriptorTypeToPoly(binding->descriptor_type),
                    binding->count
                    });
            }
        }

        // Push constants
        {
            uint32_t count = 0;
            spvReflectEnumeratePushConstants(module, & count, nullptr);
            std::vector<SpvReflectBlockVariable*> pushConstants(count);
            spvReflectEnumeratePushConstants(module, &count, pushConstants.data());

            for (auto* pushConstant : pushConstants)
            {
                result.PushConstants.push_back({
                    pushConstant->name ? pushConstant->name : "",
                    pushConstant->size,
                    pushConstant->offset
                    });
            }
        }

        return result;
	}
}
