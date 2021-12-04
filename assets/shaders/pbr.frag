#version 450
#extension GL_ARB_separate_shader_objects : enable

const float PI = 3.14159265359f;

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
	float	_UNUSED;
};

struct PointLight
{
	vec4	Color;
	vec4	Position;
};

// Sets
layout(set = 0, binding = 0) uniform Camera { mat4 camera; vec4 camPos; };
layout(set = 0, binding = 1) buffer Lights { vec4 lightsCount; PointLight pointLights[]; };

layout(set = 2, binding = 0) buffer MaterialProperties { MaterialValues material[]; };
layout(set = 2, binding = 1) uniform sampler2D albedoTex;
layout(set = 2, binding = 2) uniform sampler2D metallicTex;
layout(set = 2, binding = 3) uniform sampler2D normalTex;
layout(set = 2, binding = 4) uniform sampler2D roughnessTex;
layout(set = 2, binding = 5) uniform sampler2D aoTex;

vec3 GenerateNormal(in mat3 TBN)
{
	vec3 normal = texture(normalTex, in_TexCoord).rgb;
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
	vec3 albedo		= (material[in_MaterialIndex].Albedo * texture(albedoTex, in_TexCoord)).rgb;
	vec3 normal		= GenerateNormal(in_TBN);
	vec3 viewDir	= normalize(camPos.xyz - in_WorldPos);
	float metallic	= material[in_MaterialIndex].Metallic	* texture(metallicTex, in_TexCoord).r;
	float roughness	= material[in_MaterialIndex].Roughness	* texture(roughnessTex, in_TexCoord).r;
	float ao		= material[in_MaterialIndex].AO			* texture(aoTex, in_TexCoord).r;

	// Loop over the point lights
	vec3 Lo = vec3(0.0f);
	vec3 F0	= vec3(0.04f);
	F0		= mix(F0, albedo, metallic);
	for (uint i = 0; i < lightsCount.x; i++)
	{
		vec3 lightDir	= normalize(pointLights[i].Position.xyz - in_WorldPos);
		vec3 halfway	= normalize(viewDir + lightDir);

		float distance		= length(pointLights[i].Position.xyz - in_WorldPos);
		float attenuation	= 1.0f / (distance * distance);
		vec3 radiance		= pointLights[i].Color.xyz * attenuation;

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