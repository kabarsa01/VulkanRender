#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "CommonFrameData.glsl"
#include "CommonDepth.glsl"

layout(early_fragment_tests) in;

layout(set = 1, binding = 0) uniform texture2D frameDirectLight;
layout(set = 1, binding = 1) uniform texture2D frameGILight;
layout(set = 1, binding = 2) uniform texture2D depthTex;
layout(set = 1, binding = 3) uniform texture2D albedoTex;
layout(set = 1, binding = 4) uniform texture2D normalsTex;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	float depth = texture( sampler2D( depthTex, repeatLinearSampler ), uv ).r;
	float linearDepth = LinearizeDepth(depth, globalData.cameraNear, globalData.cameraFar);
	vec3 normal = normalize(texture( sampler2D( normalsTex, repeatLinearSampler ), uv ).xyz);
	//vec3 giLight = texture( sampler2D( frameGILight, repeatLinearSampler ), uv ).xyz;

	ivec2 depthSize = textureSize( sampler2D( depthTex, repeatLinearSampler ), 0 );
	vec2 pixelSizeUV = vec2(4.0) / depthSize;

	float minNormalCosine = cos(radians(1.5f));
	float suitableSamplesCount = 0.0f;
	vec3 filteredGI = vec3(0.0f, 0.0f, 0.0f);

	for (int x = -1; x < 2; ++x)
	{
		for (int y = -1; y < 2; ++y)
		{
			vec2 localUV = uv + (pixelSizeUV * vec2(x,y));

			float localDepth = texture( sampler2D( depthTex, repeatLinearSampler ), localUV ).r;
			float localLinearDepth = LinearizeDepth(localDepth, globalData.cameraNear, globalData.cameraFar);
			vec3 localNormal = normalize(texture( sampler2D( normalsTex, repeatLinearSampler ), localUV ).xyz);
			//vec3 localGILight = texture( sampler2D( frameGILight, repeatLinearSampler ), localUV ).xyz;

			float depthDiff = abs(linearDepth - localLinearDepth);
			//float giDiff = length(giLight - localGILight);

			if ((depthDiff <= 0.05f) && (dot(normal, localNormal) > minNormalCosine))
			{
				filteredGI += texture( sampler2D( frameGILight, repeatLinearSampler ), localUV ).xyz;
				suitableSamplesCount += 1.0f;
			}
		}
	}

	filteredGI /= suitableSamplesCount;

	vec3 albedo = texture( sampler2D( albedoTex, repeatLinearSampler ), uv ).xyz;
	vec3 directLight = texture( sampler2D( frameDirectLight, repeatLinearSampler ), uv ).xyz;
	outColor = vec4(directLight + filteredGI * albedo, 1.0f);
}
