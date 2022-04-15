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

layout(set = 1, binding = 0) uniform texture2D albedoTex;
layout(set = 1, binding = 1) uniform texture2D normalsTex;
layout(set = 1, binding = 2) uniform texture2D roghnessTex;
layout(set = 1, binding = 3) uniform texture2D metallnessTex;
layout(set = 1, binding = 4) uniform texture2D depthTex;
layout(set = 1, binding = 5) uniform utexture2D visibilityTex;

layout(set = 2, binding = 3) uniform texture2D visibilityTextures[32];

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

float GetPixelVisibility(uint index, vec2 uvCoord)
{
	vec4 visibilityInfo = texture(sampler2D( visibilityTextures[index / 4], repeatLinearSampler ), uvCoord);
	uint comp = uint(mod(index, 4u));
	if (comp == 0) return visibilityInfo.r;
	if (comp == 1) return visibilityInfo.g;
	if (comp == 2) return visibilityInfo.b;
	if (comp == 3) return visibilityInfo.a;
	return 1.0f;
}

float GetPixelVisibilityFiltered(uint index, float linearDepth, float near, float far, uint clusterIndex)
{
	float value = 0.0f;
	float counter = 0.0000001f;
	vec2 uvDelta = vec2(0.001f,0.001f);

	for (int x = -1; x < 2; x++)
	{
		for (int y = -1; y < 2; y++)
		{
			vec2 sampleUV = uv + uvDelta * vec2(x,y);
			float linearDepthSample = near * far / (far + texture(sampler2D( depthTex, repeatLinearSampler ), sampleUV).r * (near - far));
			if (abs(linearDepthSample - linearDepth) < 0.01f)
			{
				value += GetPixelVisibility(index, sampleUV);
				counter += 1.0f;
			}
		}
	}

	return value / counter;
}

void main() {
	uint clusterX = uint(gl_FragCoord.x + globalData.halfScreenOffset.x) / globalData.clusterSize.x;
	uint clusterY = uint(gl_FragCoord.y + globalData.halfScreenOffset.y) / globalData.clusterSize.y;

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
	float roughness = 0.94;//texture( sampler2D( roughnessTex, repeatLinearSampler ), uv ).r;
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
	// replace with GI
//    vec3 irradiance = vec3(0.0);//texture(irradianceMap, N).rgb;
//    vec3 diffuse = irradiance * albedo;
//    vec3 ambient = (kD * diffuse);// * AO;

	vec3 rayStart = pixelCoordWorld.xyz + (N * 0.01f);

	for (uint index = directionalOffset; index < directionalOffset + directionalCount; index++)
	{
		float visibilityFactor = GetPixelVisibilityFiltered(index, linearDepth, near, far, clusterIndex);//GetPixelVisibility(index, uv);
		if (visibilityFactor <= 0.1f)
		{
			continue;
		}

		uint lightIndicesPacked = clusterLightsData.lightIndices[clusterX][clusterY][clusterIndex][index / 2];
		LightInfo lightInfo = lightsList.lights[UnpackLightIndex(lightIndicesPacked, index)];

		vec3 pixelToLightDir = -lightInfo.direction.xyz;

		Lo += visibilityFactor * CalculateLightInfluence(albedo, N, V, F, pixelToLightDir, lightInfo.color.xyz * lightInfo.rai.z, kD, roughness);
	}
	for (uint index = spotOffset; index < spotOffset + spotCount; index++)
	{
		float visibilityFactor = GetPixelVisibilityFiltered(index, linearDepth, near, far, clusterIndex);//GetPixelVisibility(index, uv);
		if (visibilityFactor <= 0.1f)
		{
			continue;
		}

		uint lightIndicesPacked = clusterLightsData.lightIndices[clusterX][clusterY][clusterIndex][index / 2];
		LightInfo lightInfo = lightsList.lights[UnpackLightIndex(lightIndicesPacked, index)];

		vec3 pixelToLightDir = (lightInfo.position - pixelCoordWorld).xyz;

		float cosine = dot( normalize(-1.0 * pixelToLightDir), normalize(lightInfo.direction.xyz) );
		float spotCosine = cos(radians(lightInfo.rai.y));
		float angleFactor = clamp(cosine - spotCosine, 0.0, 1.0);// / clamp(1.0f - spotCosine, 0.0, 1.0);
		//float angleFactor = clamp(cosine - cos(radians(lightInfo.rai.y)), 0, 1);
		float lightRadiusSqr = lightInfo.rai.x * lightInfo.rai.x;
		float pixelDistanceSqr = dot(pixelToLightDir, pixelToLightDir);
		float distanceFactor = max(lightRadiusSqr - pixelDistanceSqr, 0) / max(lightRadiusSqr, 0.001);

		vec3 lightColor = lightInfo.color.xyz * lightInfo.rai.z * angleFactor * distanceFactor;

		Lo += visibilityFactor * CalculateLightInfluence(albedo, N, V, F, pixelToLightDir, lightColor, kD, roughness);
	}
	for (uint index = pointOffset; index < pointOffset + pointCount; index++)
	{
		float visibilityFactor = GetPixelVisibilityFiltered(index, linearDepth, near, far, clusterIndex);//GetPixelVisibility(index, uv);
		if (visibilityFactor <= 0.1f)
		{
			continue;
		}

		uint lightIndicesPacked = clusterLightsData.lightIndices[clusterX][clusterY][clusterIndex][index / 2];
		LightInfo lightInfo = lightsList.lights[UnpackLightIndex(lightIndicesPacked, index)];

		vec3 pixelToLightDir = (lightInfo.position - pixelCoordWorld).xyz;
		
		float lightRadiusSqr = lightInfo.rai.x * lightInfo.rai.x;
		float pixelDistanceSqr = dot(pixelToLightDir, pixelToLightDir);
		float distanceFactor = max(lightRadiusSqr - pixelDistanceSqr, 0) / max(lightRadiusSqr, 0.001f);

		vec3 lightColor = lightInfo.color.xyz * lightInfo.rai.z * distanceFactor;
		Lo += visibilityFactor * CalculateLightInfluence(albedo, N, V, F, pixelToLightDir, lightColor, kD, roughness);
	}

	outScreenColor = vec4(/*ambient +*/ Lo, 1.0);//vec4(normal, 1.0);//
}
