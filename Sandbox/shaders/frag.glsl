#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(set = 0, binding = 1) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
    outColor.a += texture(tex, vec2(0.0, 0.0)).r;
}