import common.bindless;

// Mirrors UIGlobalsBuffer in Poly/ImGui/ImGuiLayer.cpp byte-for-byte.
struct UIGlobals
{
	float2 scale;
	float2 translate;
}

// The fields are bound by name by the "ui" pass (see ImGuiLayer::RegisterRenderFeature). sTexture is the
// font texture by default, the pass sets it per draw call for ImGui::Image() textures.
struct PushConstants
{
	UIGlobals*    globals;
	TextureHandle sTexture;
}

[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

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
	VertexOutput output;
	output.texCoord = input.in_TexCoord;
	output.color    = input.in_Color;
	output.position = float4(input.in_Position * pc.globals.scale + pc.globals.translate, 0.0f, 1.0f);
	return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
	FragmentOutput output;
	output.out_Color = input.color * pc.sTexture.Sample(input.texCoord);
	return output;
}
