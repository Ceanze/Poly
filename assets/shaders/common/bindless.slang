// Shared bindless module - imported by every bindless-consuming shader
// Declares the global texture/sampler heap (always set 0, binding 0/1, engine-owned - matches
// BindlessManager::Init()) and the BindlessPushConstants struct every bindless push-constant
// block starts with, so textureIndices[]/bufferAddresses[] land at the same offsets in every
// shader (required by RenderPlanBuilder, which keys off the offsets SlangReflector reports).
module bindless;

public static const uint BINDLESS_MAX_SLOTS = 16;

[[vk::binding(0, 0)]] Texture2D    g_Textures[];
[[vk::binding(1, 0)]] SamplerState g_Samplers[];

public struct BindlessPushConstants
{
	public uint     textureIndices[BINDLESS_MAX_SLOTS];
	public uint64_t bufferAddresses[BINDLESS_MAX_SLOTS];
}

// Mirrors Poly::GPUInstanceData in Poly/RenderGraph/Shader/GPUInstanceData.h byte-for-byte.
public struct InstanceData
{
	public float4x4 Transform;
	public uint     MaterialIndex;
	public uint     _Pad[3];
}

// Mirrors Poly::GPUMaterialData in Poly/RenderGraph/Shader/GPUMaterialData.h byte-for-byte.
public struct MaterialValues
{
	public float4 Albedo;
	public float  AO;
	public float  Metallic;
	public float  Roughness;
	public float  IsCombined;
	public uint   TextureAlbedoIndex;
	public uint   TextureMetallicIndex;
	public uint   TextureNormalIndex;
	public uint   TextureRoughnessIndex;
	public uint   TextureAmbientOcclusionIndex;
	public uint   TextureCombinedIndex;
	public uint   _Pad[2];
}

// Unpacks a textureIndices[] entry (see BindlessManager::TEXTURE_INDEX_BITS/SAMPLER_INDEX_BITS -
// low 12 bits = texture index into g_Textures[], next 8 bits = sampler index into g_Samplers[])
// and samples it. The index is dynamically uniform per draw (comes from a push constant), but
// NonUniformResourceIndex is kept for safety per descriptor-indexing best practice.
public float4 SampleBindless(uint packed, float2 uv)
{
	uint textureIndex = packed & 0xFFFu;        // low 12 bits (TEXTURE_INDEX_BITS)
	uint samplerIndex = (packed >> 12) & 0xFFu; // next 8 bits (SAMPLER_INDEX_BITS)
	return g_Textures[NonUniformResourceIndex(textureIndex)].Sample(g_Samplers[NonUniformResourceIndex(samplerIndex)], uv);
}
