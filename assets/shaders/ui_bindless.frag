#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : require

#include "common/bindless.glsl"

// Bindless slot order - must match the "ui" pass's MapGlobal() call order:
//   textureIndices[0] = $.FontTexture (see ui_bindless.vert for bufferAddresses[0])
//
// Font-only: every draw in the pass samples the same texture, so a single push-constant slot
// resolved once per frame is enough - unlike arbitrary ImGui::Image() textures, which would need
// a per-draw-call texture index that nothing in RG2 can currently deliver (push constants are only
// (re)built once per pass per frame, before the pass's WithExecuteFn draw loop runs).

layout(location = 0) in vec2 in_TexCoord;
layout(location = 1) in vec4 in_Color;

layout(location = 0) out vec4 out_Color;

layout(push_constant, std430) uniform PushConstants
{
	BINDLESS_PUSH_CONSTANTS;
} pc;

void main()
{
	out_Color = in_Color * SampleBindless(pc.textureIndices[0], in_TexCoord);
}
