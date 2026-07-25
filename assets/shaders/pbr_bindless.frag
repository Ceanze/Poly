#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : require

#include "common/bindless.glsl"

const float PI = 3.14159265359f;

// Bindless slot order - must match this pass's ImportResource() call order (see plans/bindless.md).
// Buffer slots 0-2 are shared with pbr_bindless.vert (same push-constant block, see there):
//   bufferAddresses[0] = $.scene:Camera            (shared with vert)
//   bufferAddresses[1] = pbr_bindless.Vertices     (vert only)
//   bufferAddresses[2] = pbr_bindless.Instances    (vert only)
//   bufferAddresses[3] = $.scene:Lights
//   bufferAddresses[4] = pbr_bindless.MaterialProperties
//   textureIndices[0]  = pbr_bindless.Albedo
//   textureIndices[1]  = pbr_bindless.Metallic
//   textureIndices[2]  = pbr_bindless.Normal
//   textureIndices[3]  = pbr_bindless.Roughness
//   textureIndices[4]  = pbr_bindless.AO
//   textureIndices[5]  = pbr_bindless.Combined

// Inputs
layout(location = 0) in vec2 in_TexCoord;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_WorldPos;
layout(location = 3) in flat uint in_MaterialIndex;
layout(location = 4) in mat3 in_TBN;

// Outputs
layout(location = 0) out vec4 out_Color;

// Structs
struct MaterialValues
{
	vec4	Albedo;
	float	AO;
	float	Metallic;
	float	Roughness;
	float	IsCombined;
};

struct PointLight
{
	vec4	Color;
	vec4	Position;
};

// BDA buffer types
layout(buffer_reference, std430) readonly buffer CameraBuffer
{
	mat4 mat;
	vec4 camPos;
};

layout(buffer_reference, std430) readonly buffer LightsBuffer
{
	vec4       lightsCount;
	PointLight pointLights[];
};

layout(buffer_reference, std430) readonly buffer MaterialPropertiesBuffer
{
	MaterialValues material[];
};

layout(push_constant, std430) uniform PushConstants
{
	BINDLESS_PUSH_CONSTANTS;
} pc;

vec3 GenerateNormal(in mat3 TBN)
{
	vec3 normal = SampleBindless(pc.textureIndices[2], in_TexCoord).rgb;
	normal = normal * 2.0f - 1.0f;
	return normalize(TBN * normal);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float Distribution(vec3 normal, vec3 halfway, float roughness)
{
	float a			= roughness * roughness;
	float a2		= a * a;
	float NdotH		= max(dot(normal, halfway), 0.0f);
	float NdotH2	= NdotH * NdotH;

	float num	= a2;
	float denom	= (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlick(float NdotV, float roughness)
{
	float r = (roughness + 1.0f);
	float k = (r * r) / 8.0f;

	float num	= NdotV;
	float denom	= NdotV * (1.0f - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness)
{
	float NdotL	= max(dot(normal, lightDir), 0.0f);
	float NdotV	= max(dot(normal, viewDir), 0.0f);
	float ggx1	= GeometrySchlick(NdotL, roughness);
	float ggx2	= GeometrySchlick(NdotV, roughness);

	return ggx1 * ggx2;
}

void main()
{
	CameraBuffer             camera       = CameraBuffer(pc.bufferAddresses[0]);
	LightsBuffer             lights       = LightsBuffer(pc.bufferAddresses[3]);
	MaterialPropertiesBuffer materialProps = MaterialPropertiesBuffer(pc.bufferAddresses[4]);

	MaterialValues mat = materialProps.material[in_MaterialIndex];
	vec3 albedo		= (mat.Albedo * SampleBindless(pc.textureIndices[0], in_TexCoord)).rgb;
	vec3 normal		= GenerateNormal(in_TBN);
	vec3 viewDir	= normalize(camera.camPos.xyz - in_WorldPos);

	float metallic	= 0.f;
	float roughness	= 0.f;
	float ao		= 0.f;

	if (mat.IsCombined > 0.5f)
	{
		vec3 tex	= SampleBindless(pc.textureIndices[5], in_TexCoord).rgb;
		ao			= mat.AO		* tex.r;
		roughness	= mat.Roughness	* tex.g;
		metallic	= mat.Metallic	* tex.b;
	}
	else
	{
		metallic	= mat.Metallic	* SampleBindless(pc.textureIndices[1], in_TexCoord).r;
		roughness	= mat.Roughness	* SampleBindless(pc.textureIndices[3], in_TexCoord).r;
		ao			= mat.AO		* SampleBindless(pc.textureIndices[4], in_TexCoord).r;
	}

	// Loop over the point lights
	vec3 Lo = vec3(0.0f);
	vec3 F0	= vec3(0.04f);
	F0		= mix(F0, albedo, metallic);
	for (uint i = 0; i < lights.lightsCount.x; i++)
	{
		vec3 lightDir	= normalize(lights.pointLights[i].Position.xyz - in_WorldPos);
		vec3 halfway	= normalize(viewDir + lightDir);

		float distance		= length(lights.pointLights[i].Position.xyz - in_WorldPos);
		float attenuation	= 1.0f / (distance * distance);
		vec3 radiance		= lights.pointLights[i].Color.xyz * attenuation;

		// Fresnel, normal distribution function, and geometry
		vec3 F		= FresnelSchlick(max(dot(halfway, viewDir), 0.0f), F0);
		float NDF	= Distribution(normal, halfway, roughness);
		float G		= GeometrySmith(normal, viewDir, lightDir, roughness);

		// Cook-Torrance BRDF
		vec3 num		= NDF * G * F;
		float denom		= 4.0f * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + 0.0001;
		vec3 specular	= num / denom;

		// Reflection and refraction
		vec3 kS	= F;
		vec3 kD = vec3(1.0f) - kS;
		kD *= 1.0f - metallic;

		// Light contribution
		float NdotL = max(dot(normal, lightDir), 0.0f);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	vec3 ambient	= vec3(0.03) * albedo * ao;
	vec3 colorHDR	= ambient + Lo;
	vec3 colorLDR	= colorHDR / (colorHDR + vec3(1.0f));
	colorLDR		= pow(colorLDR, vec3(1.0f/2.2f));

	out_Color = vec4(colorLDR, 1.0f);
}
