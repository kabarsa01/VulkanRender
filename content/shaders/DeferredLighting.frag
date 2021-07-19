#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#include "CommonFrameData.glsl"
#include "CommonRay.glsl"
#include "CommonLight.glsl"

layout(early_fragment_tests) in;

//layout(push_constant) uniform PushConst
//{
//	uint transformIndexOffset;
//} pushConst;
//
//layout(set = 0, binding = 0) uniform sampler repeatLinearSampler;
//layout(set = 0, binding = 1) uniform sampler repeatMirrorLinearSampler;
//layout(set = 0, binding = 2) uniform sampler borderBlackLinearSampler;
//layout(set = 0, binding = 3) uniform sampler borderWhiteLinearSampler;
//layout(set = 0, binding = 4) uniform ShaderGlobalData
//{
//	mat4 worldToView;
//	mat4 viewToProj;
//	vec3 cameraPos;
//	vec3 viewVector;
//	float time;
//	float deltaTime;
//	float cameraNear;
//	float cameraFar;
//	float cameraFov;
//	float cameraAspect;
//} globalData;
//
//layout(set = 0, binding = 5) readonly buffer GlobalTransformData
//{
//	mat4 modelToWorld[];
//} globalTransformData;

layout(set = 1, binding = 0) uniform texture2D albedoTex;
layout(set = 1, binding = 1) uniform texture2D normalsTex;
layout(set = 1, binding = 2) uniform texture2D roghnessTex;
layout(set = 1, binding = 3) uniform texture2D metallnessTex;
layout(set = 1, binding = 4) uniform texture2D depthTex;
layout(set = 1, binding = 5) uniform utexture2D visibilityTex;

// light clustering data
layout(set = 1, binding = 6) readonly buffer ClusterLightsData
{
	uvec2 clusters[32][32][64];
	uint lightIndices[32][32][64][128];
} clusterLightsData;

// lights list data
struct LightInfo
{
	vec4 position;
	vec4 direction;
	vec4 color;
	vec4 rai;
};
layout(set = 1, binding = 7) uniform LightsList
{
	LightInfo lights[1024];
} lightsList;

layout(set = 1, binding = 8) uniform LightsIndices
{
	uvec2 directionalPosition;
	uvec2 spotPosition;
	uvec2 pointPosition;
} lightsIndices;

layout(set = 1, binding = 9) uniform accelerationStructureEXT topLevelAS;

//layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;
layout(location = 0) out vec4 outScreenColor;

bool IsVisible(uint visibility, uint index)
{
	return (visibility & (0x1 << index)) > 0;
}

void main() {
	uint clusterX = uint(clamp(uv.x * 32.0, 0, 31));
	uint clusterY = uint(clamp(uv.y * 32.0, 0, 31));

	float near = globalData.cameraNear;
	float far = globalData.cameraFar;
	float depth = texture(sampler2D( depthTex, repeatLinearSampler ), uv).r;
	if (depth == 0.0 || depth >= 1.0)
	{
		outScreenColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	uint visibility = texture(usampler2D( visibilityTex, borderBlackLinearSampler ), uv).r;

	float linearDepth = near * far / (far + depth * (near - far));
	float height = 2.0 * linearDepth * tan(radians(globalData.cameraFov * 0.5));
	float width = height * globalData.cameraAspect;
	float pixelViewSpaceX = width * (-0.5 + uv.x);
	float pixelViewSpaceY = height * (-0.5 + uv.y);

	vec4 pixelCoordWorld = inverse(globalData.worldToView) * vec4(pixelViewSpaceX, pixelViewSpaceY, -linearDepth, 1.0);
	pixelCoordWorld /= pixelCoordWorld.w;
	uint clusterIndex = clamp(uint(64.0 * log(linearDepth/near) / log(far/near)), 0, 63); // clump it just in case

	uvec2 lightsPositions = clusterLightsData.clusters[clusterX][clusterY][clusterIndex];
	uint directionalCount = (lightsPositions.x >> 8) & 0x000000ff;
	uint directionalOffset = lightsPositions.x & 0x000000ff;
	uint spotCount = (lightsPositions.x >> 24) & 0x000000ff;
	uint spotOffset = (lightsPositions.x >> 16) & 0x000000ff;
	uint pointCount = (lightsPositions.y >> 8) & 0x000000ff;
	uint pointOffset = lightsPositions.y & 0x000000ff;

	vec3 albedo = texture( sampler2D( albedoTex, repeatLinearSampler ), uv ).xyz;
	vec3 N = texture( sampler2D( normalsTex, repeatLinearSampler ), uv ).xyz;
	float roughness = 0.4;//texture( sampler2D( roughnessTex, repeatLinearSampler ), uv ).r;
	float metallness = 0.0;//texture( sampler2D( metallnessTex, repeatLinearSampler ), uv ).r;

	vec3 Lo = vec3(0.0);
	vec3 V = normalize(globalData.cameraPos - pixelCoordWorld.xyz);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallness);
    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallness;
	// some ambient lighting
    vec3 irradiance = vec3(0.01);//texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    vec3 ambient = (kD * diffuse);// * AO;

	vec3 rayStart = pixelCoordWorld.xyz + (N * 0.01f);

	for (uint index = directionalOffset; index < directionalOffset + directionalCount; index++)
	{
		uint lightIndicesPacked = clusterLightsData.lightIndices[clusterX][clusterY][clusterIndex][index / 2];
		LightInfo lightInfo = lightsList.lights[UnpackLightIndex(lightIndicesPacked, index)];

		vec3 pixelToLightDir = -lightInfo.direction.xyz;
		float surfaceCosine = dot(normalize(pixelToLightDir), N);

		if (/*surfaceCosine > 0.0f &&*/ IsVisible(visibility, index))
		{
//			isShadow = RayQueryIsShadow(topLevelAS, rayStart, normalize(pixelToLightDir.xyz), 0.1f, 150.f);
//		}
//
//		if (!isShadow)
//		{
			Lo += CalculateLightInfluence(albedo, N, V, F, pixelToLightDir, lightInfo.color.xyz * lightInfo.rai.z, kD, roughness);
		}
	}
	for (uint index = spotOffset; index < spotOffset + spotCount; index++)
	{
		uint lightIndicesPacked = clusterLightsData.lightIndices[clusterX][clusterY][clusterIndex][index / 2];
		LightInfo lightInfo = lightsList.lights[UnpackLightIndex(lightIndicesPacked, index)];

		vec3 pixelToLightDir = (lightInfo.position - pixelCoordWorld).xyz;
		float surfaceCosine = dot(normalize(pixelToLightDir), N);
		if (/*surfaceCosine > 0.0f &&*/ IsVisible(visibility, index))
		{
//			if (RayQueryIsShadow(topLevelAS, rayStart, normalize(pixelToLightDir.xyz), 0.1f, sqrt(dot(pixelToLightDir, pixelToLightDir))))
//			{
//				continue;
//			}
//		}

			float cosine = dot( normalize(-1.0 * pixelToLightDir), normalize(lightInfo.direction.xyz) );
			float angleFactor = clamp(cosine - cos(radians(lightInfo.rai.y)), 0, 1);
			float lightRadiusSqr = lightInfo.rai.x * lightInfo.rai.x;
			float pixelDistanceSqr = dot(pixelToLightDir, pixelToLightDir);
			float distanceFactor = max(lightRadiusSqr - pixelDistanceSqr, 0) / max(lightRadiusSqr, 0.001);

			vec3 lightColor = lightInfo.color.xyz * lightInfo.rai.z * angleFactor * distanceFactor;

			Lo += CalculateLightInfluence(albedo, N, V, F, pixelToLightDir, lightColor, kD, roughness);
		}
	}
	for (uint index = pointOffset; index < pointOffset + pointCount; index++)
	{
		uint lightIndicesPacked = clusterLightsData.lightIndices[clusterX][clusterY][clusterIndex][index / 2];
		LightInfo lightInfo = lightsList.lights[UnpackLightIndex(lightIndicesPacked, index)];

		vec3 pixelToLightDir = (lightInfo.position - pixelCoordWorld).xyz;
		float surfaceCosine = dot(normalize(pixelToLightDir), N);
		if (/*surfaceCosine > 0.0f &&*/ IsVisible(visibility, index))
		{
//			if (RayQueryIsShadow(topLevelAS, rayStart, normalize(pixelToLightDir.xyz), 0.1f, sqrt(dot(pixelToLightDir, pixelToLightDir))))
//			{
//				continue;
//			}
//		}

			float lightRadiusSqr = lightInfo.rai.x * lightInfo.rai.x;
			float pixelDistanceSqr = dot(pixelToLightDir, pixelToLightDir);
			float distanceFactor = max(lightRadiusSqr - pixelDistanceSqr, 0) / max(lightRadiusSqr, 0.001f);

			vec3 lightColor = lightInfo.color.xyz * lightInfo.rai.z * distanceFactor;
			Lo += CalculateLightInfluence(albedo, N, V, F, pixelToLightDir, lightColor, kD, roughness);
		}
	}

	outScreenColor = vec4(ambient + Lo, 1.0);//vec4(normal, 1.0);//
}
