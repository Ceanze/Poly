// Shared bindless module - imported by every bindless-consuming shader
// Declares the global texture/sampler heap (always set 0, binding 0/1, engine-owned - matches
// BindlessManager::Init()) and TextureHandle, the type of a texture in a push-constant block.
//
// A pass gets its resources through its push constants: a pointer field for a buffer, a TextureHandle
// field for a texture. RenderPlanBuilder fills each field with the resource the pass mapped to the
// name of the field (MapGlobal/ImportResource/ExportResource), so the order of the fields is free.
module bindless;

[[vk::binding(0, 0)]] Texture2D    g_Textures[];
[[vk::binding(1, 0)]] SamplerState g_Samplers[];

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

// Unpacks a packed texture/sampler index (see ResourceManager::TEXTURE_INDEX_BITS/SAMPLER_INDEX_BITS -
// low 12 bits = texture index into g_Textures[], next 8 bits = sampler index into g_Samplers[])
// and samples it. NonUniformResourceIndex is kept for safety per descriptor-indexing best practice,
// as the index can come from per-instance data (e.g. MaterialValues).
public float4 SampleBindless(uint packed, float2 uv)
{
	uint textureIndex = packed & 0xFFFu;        // low 12 bits (TEXTURE_INDEX_BITS)
	uint samplerIndex = (packed >> 12) & 0xFFu; // next 8 bits (SAMPLER_INDEX_BITS)
	return g_Textures[NonUniformResourceIndex(textureIndex)].Sample(g_Samplers[NonUniformResourceIndex(samplerIndex)], uv);
}

// A texture in a push-constant block. The reflection recognizes the field by this type, so the name of
// the struct is load bearing (see SlangReflector)
public struct TextureHandle
{
	uint packed;

	public float4 Sample(float2 uv) { return SampleBindless(packed, uv); }
}
