#pragma once

namespace Poly
{
    struct ShaderInput
    {
        std::string Name;
        uint32      Location;
    };

    struct ShaderBinding
    {
        std::string         Name;
        uint32              Set;
        uint32              Binding;
        FResourceBindPoint  DescriptorType;
        uint32              Count;           // array size
    };

    struct ShaderPushConstant
    {
        std::string Name;
        uint32      Size;
        uint32      Offset;
    };

    struct ShaderReflection
    {
        std::vector<ShaderInput>        Inputs;
        std::vector<ShaderBinding>      Bindings;
        std::vector<ShaderPushConstant> PushConstants;
    };
}