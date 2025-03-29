#version 450
#extension GL_ARB_separate_shader_objects : enable

// Outputs
layout(location = 0) out vec2 out_TexCoord;
layout(location = 1) out vec3 out_Normal;
layout(location = 2) out vec3 out_WorldPos;
layout(location = 3) out flat uint out_MaterialIndex;
layout(location = 4) out mat3 out_TBN;

// Structs
struct Vertex
{
	vec4 Position;
	vec4 Normal;
	vec4 Tangent;
	vec4 TexCoord;
};

// Sets
layout(set = 0, binding = 0) uniform Camera { mat4 camera; vec4 camPos; };
layout(set = 4, binding = 0) buffer Vertices { Vertex vertex[]; };
layout(set = 1, binding = 0) buffer Transforms { mat4 transform[]; };

void main() {
	vec4 worldPosition = transform[gl_InstanceIndex] * vec4(vertex[gl_VertexIndex].Position.xyz, 1.0f);

	vec3 normal		= normalize(transform[gl_InstanceIndex] * vertex[gl_VertexIndex].Normal).xyz;
	vec3 tangent	= normalize(transform[gl_InstanceIndex] * vertex[gl_VertexIndex].Tangent).xyz;
	vec3 bitangent	= normalize(cross(normal, tangent));
	mat3 TBN		= mat3(tangent, bitangent, normal);

	out_TexCoord		= vertex[gl_VertexIndex].TexCoord.xy;
	out_Normal			= vertex[gl_VertexIndex].Normal.xyz;
	out_TBN				= TBN;
	out_WorldPos		= worldPosition.xyz;
	out_MaterialIndex	= gl_InstanceIndex;

	gl_Position = camera * worldPosition;
}