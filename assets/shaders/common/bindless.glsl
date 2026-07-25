// Shared bindless header - included by every bindless-consuming shader (see plans/bindless.md).
// Declares the global texture/sampler heap (always set 0, binding 0/1, engine-owned - matches
// BindlessManager::Init()) and the BINDLESS_PUSH_CONSTANTS macro every bindless push-constant
// block starts with, so textureIndices[]/bufferAddresses[] land at the same offsets in every
// shader (required by RenderProgramBuilder/ShaderReflector, which key off those fixed offsets).

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2    : require
#extension GL_ARB_gpu_shader_int64     : require // uint64_t scalar type, used by bufferAddresses[]

#define BINDLESS_MAX_SLOTS 16

layout(set = 0, binding = 0) uniform texture2D g_Textures[];
layout(set = 0, binding = 1) uniform sampler   g_Samplers[];

#define BINDLESS_PUSH_CONSTANTS \
	uint     textureIndices[BINDLESS_MAX_SLOTS]; \
	uint64_t bufferAddresses[BINDLESS_MAX_SLOTS]

// Unpacks a textureIndices[] entry (see BindlessManager::TEXTURE_INDEX_BITS/SAMPLER_INDEX_BITS -
// low 12 bits = texture index into g_Textures[], next 8 bits = sampler index into g_Samplers[])
// and samples it. The index is dynamically uniform per draw (comes from a push constant), but
// nonuniformEXT is kept for safety per descriptor-indexing best practice.
vec4 SampleBindless(uint packed, vec2 uv)
{
	uint textureIndex = packed & 0xFFFu;        // low 12 bits (TEXTURE_INDEX_BITS)
	uint samplerIndex = (packed >> 12) & 0xFFu; // next 8 bits (SAMPLER_INDEX_BITS)
	return texture(sampler2D(g_Textures[nonuniformEXT(textureIndex)], g_Samplers[nonuniformEXT(samplerIndex)]), uv);
}
