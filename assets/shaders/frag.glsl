#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 1) in vec2 fragUV;
layout(set = 2, binding = 0) uniform sampler2D meshTexture;

layout(location = 0) out vec4 outColor;

void main() {
    // outColor = vec4(fragColor, 1.0);
    outColor = texture(meshTexture, fragUV);
}