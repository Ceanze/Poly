import common.bindless;

static const float PI = 3.14159265359f;

// Bindless slot order - must match this pass's MapGlobal() call order (see plans/bindless.md,
// "the two sides stay in sync only by matching declaration order"). Both stages read the same
// push-constant block:
//   bufferAddresses[0] = view.camera
//   bufferAddresses[1] = scene.vertices    (vertex only)
//   bufferAddresses[2] = scene.instances   (vertex only)
//   bufferAddresses[3] = scene.lights      (fragment only)
//   bufferAddresses[4] = scene.materials   (fragment only)
//
// Vertices/Instances are combined, scene-wide buffers built by RenderSystem (see
// Poly/World/Systems/RenderSystem.h) - which mesh/instances a draw call touches comes from
// DrawIndexedInstanced's own baseVertex/firstInstance parameters, not from anything in this push
// constant, so the buffer addresses here never need to change between draw calls in the pass.
//
// Per-material texture indices live inside each MaterialValues row, indexed per-draw via the
// material index of the instance - push constants only get built once per pass, before the draw
// loop runs, so they cannot carry per-submesh textures (see plans/missing_for_rg2.md).

// Mirrors Poly::Vertex in Poly/Model/Mesh.h byte-for-byte.
struct Vertex
{
	float4 Position;
	float4 Normal;
	float4 Tangent;
	float4 TexCoord;
}

// Mirrors Poly::GPUCameraData in Poly/RenderGraph/Shader/GPUCameraData.h byte-for-byte.
struct Camera
{
	float4x4 mat;
	float4   camPos;
}

// Mirrors Poly::GPUPointLight in Poly/RenderGraph/Shader/GPULightData.h byte-for-byte.
struct PointLight
{
	float4 Color;
	float4 Position;
}

// Mirrors Poly::GPULightsHeader, the lights buffer is one header followed by lightsCount.x PointLights.
struct LightsHeader
{
	float4 lightsCount;
}

[[vk::push_constant]] ConstantBuffer<BindlessPushConstants> pc;

struct VertexOutput
{
	float4               position      : SV_Position;
	float2               texCoord      : TEXCOORD0;
	float3               normal        : NORMAL0;
	float3               worldPos      : POSITION0;
	nointerpolation uint materialIndex : MATERIAL0;
	float3               tangent       : TANGENT0;
	float3               bitangent     : BITANGENT0;
	float3               tbnNormal     : NORMAL1;
}

struct FragmentOutput
{
	float4 out_Color : SV_Target0;
}

// The Vulkan flavored IDs include the baseVertex/firstInstance of the draw call, which is what indexes the scene-wide buffers
[shader("vertex")]
VertexOutput vertexMain(uint vertexIndex : SV_VulkanVertexID, uint instanceIndex : SV_VulkanInstanceID)
{
	Camera*       pCamera    = (Camera*)pc.bufferAddresses[0];
	Vertex*       pVertices  = (Vertex*)pc.bufferAddresses[1];
	InstanceData* pInstances = (InstanceData*)pc.bufferAddresses[2];

	InstanceData instanceData = pInstances[instanceIndex];
	Vertex       vertex       = pVertices[vertexIndex];

	float4 worldPosition = mul(instanceData.Transform, float4(vertex.Position.xyz, 1.0f));

	float3 normal    = normalize(mul(instanceData.Transform, vertex.Normal)).xyz;
	float3 tangent   = normalize(mul(instanceData.Transform, vertex.Tangent)).xyz;
	float3 bitangent = normalize(cross(normal, tangent));

	VertexOutput output;
	output.texCoord      = vertex.TexCoord.xy;
	output.normal        = vertex.Normal.xyz;
	output.worldPos      = worldPosition.xyz;
	output.materialIndex = instanceData.MaterialIndex;
	output.tangent       = tangent;
	output.bitangent     = bitangent;
	output.tbnNormal     = normal;
	output.position      = mul(pCamera.mat, worldPosition);
	return output;
}

float3 GenerateNormal(VertexOutput input, uint normalTexIndex)
{
	float3 normal = SampleBindless(normalTexIndex, input.texCoord).rgb;
	normal        = normal * 2.0f - 1.0f;
	// Tangent space to world space, with tangent/bitangent/normal as the basis
	return normalize(input.tangent * normal.x + input.bitangent * normal.y + input.tbnNormal * normal.z);
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float Distribution(float3 normal, float3 halfway, float roughness)
{
	float a      = roughness * roughness;
	float a2     = a * a;
	float NdotH  = max(dot(normal, halfway), 0.0f);
	float NdotH2 = NdotH * NdotH;

	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom       = PI * denom * denom;

	return num / denom;
}

float GeometrySchlick(float NdotV, float roughness)
{
	float r = (roughness + 1.0f);
	float k = (r * r) / 8.0f;

	float num   = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return num / denom;
}

float GeometrySmith(float3 normal, float3 viewDir, float3 lightDir, float roughness)
{
	float NdotL = max(dot(normal, lightDir), 0.0f);
	float NdotV = max(dot(normal, viewDir), 0.0f);
	float ggx1  = GeometrySchlick(NdotL, roughness);
	float ggx2  = GeometrySchlick(NdotV, roughness);

	return ggx1 * ggx2;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
	Camera*         pCamera       = (Camera*)pc.bufferAddresses[0];
	LightsHeader*   pLightsHeader = (LightsHeader*)pc.bufferAddresses[3];
	PointLight*     pPointLights  = (PointLight*)(pc.bufferAddresses[3] + sizeof(LightsHeader));
	MaterialValues* pMaterials    = (MaterialValues*)pc.bufferAddresses[4];

	MaterialValues mat     = pMaterials[input.materialIndex];
	float3         albedo  = (mat.Albedo * SampleBindless(mat.TextureAlbedoIndex, input.texCoord)).rgb;
	float3         normal  = GenerateNormal(input, mat.TextureNormalIndex);
	float3         viewDir = normalize(pCamera.camPos.xyz - input.worldPos);

	float metallic  = 0.0f;
	float roughness = 0.0f;
	float ao        = 0.0f;

	if (mat.IsCombined > 0.5f)
	{
		float3 tex = SampleBindless(mat.TextureCombinedIndex, input.texCoord).rgb;
		ao         = mat.AO * tex.r;
		roughness  = mat.Roughness * tex.g;
		metallic   = mat.Metallic * tex.b;
	}
	else
	{
		metallic  = mat.Metallic * SampleBindless(mat.TextureMetallicIndex, input.texCoord).r;
		roughness = mat.Roughness * SampleBindless(mat.TextureRoughnessIndex, input.texCoord).r;
		ao        = mat.AO * SampleBindless(mat.TextureAmbientOcclusionIndex, input.texCoord).r;
	}

	// Loop over the point lights
	float3 Lo = float3(0.0f);
	float3 F0 = float3(0.04f);
	F0        = lerp(F0, albedo, metallic);
	for (uint i = 0; i < pLightsHeader.lightsCount.x; i++)
	{
		PointLight light = pPointLights[i];

		float3 lightDir = normalize(light.Position.xyz - input.worldPos);
		float3 halfway  = normalize(viewDir + lightDir);

		float  distance    = length(light.Position.xyz - input.worldPos);
		float  attenuation = 1.0f / (distance * distance);
		float3 radiance    = light.Color.xyz * attenuation;

		// Fresnel, normal distribution function, and geometry
		float3 F   = FresnelSchlick(max(dot(halfway, viewDir), 0.0f), F0);
		float  NDF = Distribution(normal, halfway, roughness);
		float  G   = GeometrySmith(normal, viewDir, lightDir, roughness);

		// Cook-Torrance BRDF
		float3 num      = NDF * G * F;
		float  denom    = 4.0f * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + 0.0001;
		float3 specular = num / denom;

		// Reflection and refraction
		float3 kS = F;
		float3 kD = float3(1.0f) - kS;
		kD *= 1.0f - metallic;

		// Light contribution
		float NdotL = max(dot(normal, lightDir), 0.0f);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	float3 ambient  = float3(0.03) * albedo * ao;
	float3 colorHDR = ambient + Lo;
	float3 colorLDR = colorHDR / (colorHDR + float3(1.0f));
	colorLDR        = pow(colorLDR, float3(1.0f / 2.2f));

	FragmentOutput output;
	output.out_Color = float4(colorLDR, 1.0f);
	return output;
}
