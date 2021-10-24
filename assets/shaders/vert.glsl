#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;

// vec2 positions[3] = vec2[](
//     vec2(-0.5, -0.5),
//     vec2(0.5, -0.5),
//     vec2(0.0, 0.5)
// );

// vec3 colors[3] = vec3[](
//     vec3(1.0, 0.0, 0.0),
//     vec3(0.0, 1.0, 0.0),
//     vec3(0.0, 0.0, 1.0)
// );

// vec2 uvs[3] = vec2[](
//     vec2(0.0, 0.0),
//     vec2(1.0, 0.0),
//     vec2(0.5, 1.0)
// );

struct Vertex
{
    vec4 Position;
    vec4 Normal;
    vec4 Tangent;
    vec4 TexCoord;
};

layout(set = 0, binding = 0) uniform Camera
{
    mat4 camera;
};

layout(set = 1, binding = 0) buffer Vertices
{
    Vertex vertex[];
};

void main() {
    // gl_Position = camera * vec4(positions[gl_VertexIndex], 0.0, 1.0);
    // fragColor = colors[gl_VertexIndex];
    // fragUV = uvs[gl_VertexIndex];
    gl_Position = camera * vec4(vertex[gl_VertexIndex].Position.xyz, 1.0);
    fragColor = vec3(1.0, 0.0, 0.0);
    fragUV = vec2(0.0, 0.0);
}