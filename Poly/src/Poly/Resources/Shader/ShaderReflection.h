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
        std::vector<ShaderInputOutput>  Inputs;
        std::vector<ShaderInputOutput>  Outputs;
        std::vector<ShaderBinding>      Bindings;
        std::vector<ShaderPushConstant> PushConstants;
    };
}