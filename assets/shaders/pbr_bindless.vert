#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : require

#include "common/bindless.glsl"

// Bindless slot order - must match this pass's ImportResource() call order (see plans/bindless.md,
// "the two sides stay in sync only by matching declaration order"). Buffer slots are shared with
// pbr_bindless.frag since both stages read the same push-constant block:
//   bufferAddresses[0] = $.scene:Camera
//   bufferAddresses[1] = pbr_bindless.Vertices
//   bufferAddresses[2] = pbr_bindless.Instances
//   bufferAddresses[3.. ] = used by pbr_bindless.frag only, see there
//
// Vertices/Instances are combined, scene-wide buffers built by SceneRenderBridge (see
// Poly/RenderGraph/SceneRenderBridge.h) - which mesh/instances a draw call touches comes from
// DrawIndexedInstanced's own baseVertex/firstInstance parameters, not from anything in this push
// constant, so the buffer addresses here never need to change between draw calls in the pass.

// Structs
struct Vertex
{
	vec4 Position;
	vec4 Normal;
	vec4 Tangent;
	vec4 TexCoord;
};

// BDA buffer types
layout(buffer_reference, std430) readonly buffer CameraBuffer
{
	mat4 mat;
	vec4 camPos;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer
{
	Vertex vertex[];
};

layout(buffer_reference, std430) readonly buffer InstanceBuffer
{
	InstanceData data[];
};

layout(push_constant, std430) uniform PushConstants
{
	BINDLESS_PUSH_CONSTANTS;
} pc;

// Outputs
layout(location = 0) out vec2 out_TexCoord;
layout(location = 1) out vec3 out_Normal;
layout(location = 2) out vec3 out_WorldPos;
layout(location = 3) out flat uint out_MaterialIndex;
layout(location = 4) out mat3 out_TBN;

void main() {
	CameraBuffer   camera    = CameraBuffer(pc.bufferAddresses[0]);
	VertexBuffer   vertices  = VertexBuffer(pc.bufferAddresses[1]);
	InstanceBuffer instances = InstanceBuffer(pc.bufferAddresses[2]);

	InstanceData instanceData = instances.data[gl_InstanceIndex];

	vec4 worldPosition = instanceData.Transform * vec4(vertices.vertex[gl_VertexIndex].Position.xyz, 1.0f);

	vec3 normal		= normalize(instanceData.Transform * vertices.vertex[gl_VertexIndex].Normal).xyz;
	vec3 tangent	= normalize(instanceData.Transform * vertices.vertex[gl_VertexIndex].Tangent).xyz;
	vec3 bitangent	= normalize(cross(normal, tangent));
	mat3 TBN		= mat3(tangent, bitangent, normal);

	out_TexCoord		= vertices.vertex[gl_VertexIndex].TexCoord.xy;
	out_Normal			= vertices.vertex[gl_VertexIndex].Normal.xyz;
	out_TBN				= TBN;
	out_WorldPos		= worldPosition.xyz;
	out_MaterialIndex	= instanceData.MaterialIndex;

	gl_Position = camera.mat * worldPosition;
}
