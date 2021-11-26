#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec4 albedoColor;

layout(set = 2, binding = 1) uniform sampler2D albedoTex;
layout(set = 2, binding = 2) uniform sampler2D metalicTex;
layout(set = 2, binding = 3) uniform sampler2D normalTex;
layout(set = 2, binding = 4) uniform sampler2D roughnessTex;
layout(set = 2, binding = 5) uniform sampler2D aoTex;


layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(albedoTex, fragUV) * albedoColor;
}