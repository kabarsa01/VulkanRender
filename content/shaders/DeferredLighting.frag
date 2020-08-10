#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

layout(push_constant) uniform PushConst
{
	uint transformIndexOffset;
} pushConst;

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

layout(set = 0, binding = 5) readonly buffer GlobalTransformData
{
	mat4 modelToWorld[];
} globalTransformData;

layout(set = 1, binding = 0) uniform texture2D albedoTex;
layout(set = 1, binding = 1) uniform texture2D normalsTex;
layout(set = 1, binding = 2) uniform texture2D roghnessTex;
layout(set = 1, binding = 3) uniform texture2D metallnessTex;
layout(set = 1, binding = 4) uniform texture2D depthTex;

// Pi =)
const float PI = 3.14159265359;

// light clustering data
layout(set = 1, binding = 5) readonly buffer ClusterLightsData
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
layout(set = 1, binding = 6) uniform LightsList
{
	LightInfo lights[1024];
} lightsList;

layout(set = 1, binding = 7) uniform LightsIndices
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

vec3 CalculateSpec(vec3 inViewDir, vec3 inLightDir, vec3 inNormal, vec3 inSpecColor, float inSpecStrength)
{
    // blinn-phong intermediate vector and spec value calculation
	vec3 intermediate = normalize(inViewDir + inLightDir);
	return inSpecColor * pow(max(dot(intermediate, inNormal), 0.0), 32) * inSpecStrength;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 CalculateLightInfluence(vec3 albedo, vec3 N, vec3 V, vec3 F, vec3 inPixelToLightDir, vec3 inLightColor, vec3 kD, float roughness)
{
		vec3 L = normalize(inPixelToLightDir);
        vec3 H = normalize(V + L);

        float attenuation = 1.0;
        vec3 radiance = inLightColor * attenuation;// * (1.0 - shadowAttenuation);

        float NDF = DistributionGGX(N, H, roughness);  
        float G = GeometrySmith(N, V, L, roughness);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = numerator / max(denominator, 0.001);
  
        float NdotL = max(dot(N, L), 0.0);        
        return (kD * albedo / PI + specular) * radiance * NdotL;
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

	float linearDepth = 2.0 * near * far / (far + depth * (near - far));
	float width = 2.0 * linearDepth * tan(radians(globalData.cameraFov * 0.5));
	float height = width / globalData.cameraAspect;
	float pixelViewSpaceX = width * (0.5 - uv.x);
	float pixelViewSpaceY = height * (-0.5 + uv.y);

	vec4 pixelCoordWorld = inverse(globalData.worldToView) * vec4(-pixelViewSpaceX, pixelViewSpaceY, -linearDepth, 1.0);
	float normalizedDepth = (linearDepth - near) / (far - near);
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
	float roughness = 0.2;//texture( sampler2D( roughnessTex, repeatLinearSampler ), uv ).r;
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
    vec3 irradiance = vec3(0.03);//texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    vec3 ambient = (kD * diffuse);// * AO;

	for (uint index = directionalOffset; index < directionalOffset + directionalCount; index++)
	{
		uint lightIndicesPacked = clusterLightsData.lightIndices[clusterX][clusterY][clusterIndex][index / 2];
		LightInfo lightInfo = lightsList.lights[UnpackLightIndex(lightIndicesPacked, index)];

		vec3 pixelToLightDir = -lightInfo.direction.xyz;
		float surfaceCosine = max(dot(normalize(pixelToLightDir), normal), 0.0);

		Lo += CalculateLightInfluence(albedo, N, V, F, pixelToLightDir, lightInfo.color.xyz * lightInfo.rai.z, kD, roughness);
	}
	for (uint index = spotOffset; index < spotOffset + spotCount; index++)
	{
		uint lightIndicesPacked = clusterLightsData.lightIndices[clusterX][clusterY][clusterIndex][index / 2];
		LightInfo lightInfo = lightsList.lights[UnpackLightIndex(lightIndicesPacked, index)];

		vec3 pixelToLightDir = (lightInfo.position - pixelCoordWorld).xyz;

		float cosine = dot( normalize(-1.0 * pixelToLightDir), normalize(lightInfo.direction.xyz) );
		float angleFactor = clamp(cosine - cos(radians(lightInfo.rai.y)), 0, 1);
		float lightRadiusSqr = lightInfo.rai.x * lightInfo.rai.x;
		float pixelDistanceSqr = dot(pixelToLightDir, pixelToLightDir);
		float distanceFactor = max(lightRadiusSqr - pixelDistanceSqr, 0) / max(lightRadiusSqr, 0.001);

		vec3 lightColor = lightInfo.color.xyz * lightInfo.rai.z * angleFactor * distanceFactor;

		Lo += CalculateLightInfluence(albedo, N, V, F, pixelToLightDir, lightColor, kD, roughness);
	}
	for (uint index = pointOffset; index < pointOffset + pointCount; index++)
	{
		uint lightIndicesPacked = clusterLightsData.lightIndices[clusterX][clusterY][clusterIndex][index / 2];
		LightInfo lightInfo = lightsList.lights[UnpackLightIndex(lightIndicesPacked, index)];

		vec3 pixelToLightDir = (lightInfo.position - pixelCoordWorld).xyz;
		float lightRadiusSqr = lightInfo.rai.x * lightInfo.rai.x;
		float pixelDistanceSqr = dot(pixelToLightDir, pixelToLightDir);
		float distanceFactor = max(lightRadiusSqr - pixelDistanceSqr, 0) / lightRadiusSqr;
		float surfaceCosine = max(dot(normalize(pixelToLightDir), normal), 0.0);

		vec3 lightColor = lightInfo.color.xyz * lightInfo.rai.z * distanceFactor;
		Lo += CalculateLightInfluence(albedo, N, V, F, pixelToLightDir, lightColor, kD, roughness);
	}

	outScreenColor = vec4(ambient + Lo, 1.0);//vec4(normal, 1.0);//
}
