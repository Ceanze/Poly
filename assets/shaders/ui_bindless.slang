import common.bindless;

// Bindless slot order - must match the "ui" pass's MapGlobal() call order:
//   bufferAddresses[0] = $.UIGlobals (scale/translate)
//   textureIndices[0]  = $.FontTexture
//
// Font-only: every draw in the pass samples the same texture, so a single push-constant slot
// resolved once per frame is enough - unlike arbitrary ImGui::Image() textures, which would need
// a per-draw-call texture index that nothing in RG2 can currently deliver (push constants are only
// (re)built once per pass per frame, before the pass's WithExecuteFn draw loop runs).

// Mirrors UIGlobalsBuffer in Poly/ImGui/ImGuiLayer.cpp byte-for-byte.
struct UIGlobals
{
	float2 scale;
	float2 translate;
}

[[vk::push_constant]] ConstantBuffer<BindlessPushConstants> pc;

struct VertexInput
{
	float2 in_Position : POSITION;
	float2 in_TexCoord : TEXCOORD0;
	float4 in_Color    : COLOR0;
}

struct VertexOutput
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
	float4 color    : COLOR0;
}

struct FragmentOutput
{
	float4 out_Color : SV_Target0;
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
	UIGlobals* pGlobals = (UIGlobals*)pc.bufferAddresses[0];

	VertexOutput output;
	output.texCoord = input.in_TexCoord;
	output.color    = input.in_Color;
	output.position = float4(input.in_Position * pGlobals.scale + pGlobals.translate, 0.0f, 1.0f);
	return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
	FragmentOutput output;
	output.out_Color = input.color * SampleBindless(pc.textureIndices[0], input.texCoord);
	return output;
}
