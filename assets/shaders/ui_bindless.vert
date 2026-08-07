#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : require

#include "common/bindless.glsl"

// Bindless slot order - must match the "ui" pass's MapGlobal() call order:
//   bufferAddresses[0] = $.UIGlobals (scale/translate, see ui_bindless.frag for textureIndices[0])

layout(buffer_reference, std430) readonly buffer UIGlobalsBuffer
{
	vec2 scale;
	vec2 translate;
};

layout(push_constant, std430) uniform PushConstants
{
	BINDLESS_PUSH_CONSTANTS;
} pc;

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec4 in_Color;

layout(location = 0) out vec2 out_TexCoord;
layout(location = 1) out vec4 out_Color;

void main()
{
	UIGlobalsBuffer globals = UIGlobalsBuffer(pc.bufferAddresses[0]);

	out_TexCoord = in_TexCoord;
	out_Color    = in_Color;

	gl_Position = vec4(in_Position * globals.scale + globals.translate, 0.0f, 1.0f);
}
