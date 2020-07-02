#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler repeatLinearSampler;
layout(set = 0, binding = 1) uniform sampler repeatMirrorLinearSampler;
layout(set = 0, binding = 2) uniform sampler borderBlackLinearSampler;
layout(set = 0, binding = 3) uniform sampler borderWhiteLinearSampler;
layout(set = 0, binding = 4) uniform ShaderGlobalData
{
	mat4 worldToView;
	mat4 viewToProj;
	vec3 cameraPos;
	vec3 viewVector;
	float time;
	float deltaTime;
	float cameraNear;
	float cameraFar;
	float cameraFov;
	float cameraAspect;
} globalData;

layout(set = 1, binding = 1) uniform texture2D albedoTex;
layout(set = 1, binding = 2) uniform texture2D normalsTex;
layout(set = 1, binding = 3) uniform texture2D depthTexture;

// light clustering data
layout(set = 1, binding = 4) buffer ClusterLightsData
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
layout(set = 1, binding = 5) uniform LightsList
{
	LightInfo lights[1024];
} lightsList;

layout(set = 1, binding = 6) uniform LightsIndices
{
	uvec2 directionalPosition;
	uvec2 spotPosition;
	uvec2 pointPosition;
} lightsIndices;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outScreenColor;

uint UnpackLightIndex(uint packedIndices, uint indexPosition)
{
	uint bitOffset = 16 * (indexPosition % 2);
	return (packedIndices >> bitOffset) & 0x0000ffff;
}

void main() {
	uint clusterX = uint(clamp(uv.x * 32.0, 0, 31));
	uint clusterY = uint(clamp(uv.y * 32.0, 0, 31));

	float near = globalData.cameraNear;
	float far = globalData.cameraFar;
	float depth = texture(sampler2D( depthTexture, repeatLinearSampler ), uv).r;
	if (depth == 0.0 || depth >= 1.0)
	{
		outScreenColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	float linearDepth = 2.0 * near * far / (far + depth * (near - far));
	float width = 2.0 * linearDepth * tan(radians(globalData.cameraFov * 0.5));
	float height = width / globalData.cameraAspect;
	float pixelViewSpaceX = width * (0.5 - uv.x);
	float pixelViewSpaceY = height * (-0.5 + uv.x);

	vec4 pixelCoordWorld = inverse(globalData.worldToView) * vec4(pixelViewSpaceX, pixelViewSpaceY, linearDepth, 1.0);
	float normalizedDepth = (linearDepth - near) / (far - near);
	uint clusterIndex = clamp(uint(64.0 * log(linearDepth/near) / log(far/near)), 0, 63); // clump it just in case

	uvec2 lightsPositions = clusterLightsData.clusters[clusterX][clusterY][clusterIndex];
	uint spotCount = (lightsPositions.x >> 24) & 0x000000ff;
	uint spotOffset = (lightsPositions.x >> 16) & 0x000000ff;
	uint pointCount = (lightsPositions.y >> 8) & 0x000000ff;
	uint pointOffset = lightsPositions.y & 0x000000ff;

	vec4 albedo = texture( sampler2D( albedoTex, repeatLinearSampler ), uv );
	vec3 accumulatedLight = vec3(0.0, 0.0, 0.0);

	for (uint index = spotOffset; index < spotOffset + spotCount; index++)
	{
		uint lightIndicesPacked = clusterLightsData.lightIndices[clusterX][clusterY][clusterIndex][index / 2];
		LightInfo lightInfo = lightsList.lights[UnpackLightIndex(lightIndicesPacked, index)];
	}
	for (uint index = pointOffset; index < pointOffset + pointCount; index++)
	{
		uint lightIndicesPacked = clusterLightsData.lightIndices[clusterX][clusterY][clusterIndex][index / 2];
		LightInfo lightInfo = lightsList.lights[UnpackLightIndex(lightIndicesPacked, index)];
	}

	if (spotCount + pointCount > 0)
	{
		float f = float(spotCount + pointCount) / 2.0;
		outScreenColor = vec4(f,f,f,1.0);//albedo;// * vec4(float(clusterX) / 32.0, float(clusterY) / 32.0, float(clusterIndex) / 32.0, 1.0);
	}
	else
	{
		outScreenColor = vec4(0.2, 0.0, 0.0, 1.0);
	}

	//outScreenColor = vec4(clustIdx, clustIdx, clustIdx, 1.0);
	//outScreenColor = vec4(float(pointDist) / 100.0, float(pointDist) / 100.0, float(pointDist) / 100.0, 1.0);
	//outScreenColor = vec4(float(clusterX) / 32.0, float(clusterY) / 32.0, float(clusterIndex) / 32.0, 1.0);
	//outScreenColor = vec4(normalizedDepth, 0.0, 0.0, 1.0);
	//outScreenColor = vec4(uv, 0.0, 1.0);
    //outScreenColor = texture( sampler2D( albedoTex, repeatLinearSampler ), uv );
}
