#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec4 albedoColor;

struct Vertex
{
    vec4 Position;
    vec4 Normal;
    vec4 Tangent;
    vec4 TexCoord;
};

struct MaterialValues
{
	vec4	Albedo;
	float	AO;
	float	Metallic;
	float	Roughness;
	float	_UNUSED;
};


layout(set = 0, binding = 0) uniform Camera
{
    mat4 camera;
};

layout(set = 1, binding = 0) buffer Vertices
{
    Vertex vertex[];
};

layout(set = 1, binding = 1) buffer Transforms
{
    mat4 transform[];
};

layout(set = 2, binding = 1) buffer MaterialProperties
{
	MaterialValues material[];
};

void main() {
    gl_Position = camera * transform[gl_InstanceIndex] * vec4(vertex[gl_VertexIndex].Position.xyz, 1.0);
    fragUV = vertex[gl_VertexIndex].TexCoord.xy;
    albedoColor = material[gl_InstanceIndex].Albedo;
}