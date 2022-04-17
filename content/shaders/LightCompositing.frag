#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "CommonFrameData.glsl"
#include "CommonDepth.glsl"
#include "CommonRay.glsl"

layout(early_fragment_tests) in;

layout(set = 1, binding = 0) uniform texture2D frameDirectLight;
layout(set = 1, binding = 1) uniform texture2D giLight;
layout(set = 1, binding = 2) uniform texture2D depthTex;
layout(set = 1, binding = 3) uniform texture2D albedoTex;
layout(set = 1, binding = 4) uniform texture2D normalsTex;

layout(set = 2, binding = 0) buffer ProbesBuffer
{
	DDGIProbe probes[32][16][32];
} probesBuffer;
layout(set = 2, binding = 1) uniform texture2D probesImage;
layout(set = 2, binding = 2) uniform texture2D probesDepthImage;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	float depth = texture( sampler2D( depthTex, borderBlackNearestSampler ), uv ).r;
	float linearDepth = LinearizeDepth(depth, globalData.cameraNear, globalData.cameraFar);
	vec4 worldPos = CalculateWorldPosition(linearDepth, globalData.cameraFov, globalData.cameraAspect, uv, globalData.worldToView);
	vec3 normal = normalize(texture( sampler2D( normalsTex, repeatLinearSampler ), uv ).xyz);
	//vec3 normal = normalize(vec3(0.00, -1.0, 0.00));
	//vec3 giLight = texture( sampler2D( frameGILight, repeatLinearSampler ), uv ).xyz;
	//ivec2 giImageSize = imageSize(frameGILight);

	//ivec2 depthSize = textureSize( sampler2D( depthTex, repeatLinearSampler ), 0 );
	ivec2 texSize = textureSize( sampler2D( depthTex, repeatLinearSampler ), 0 );
	vec2 pixelSizeUV = vec2(1.0) / texSize;//depthSize;

	float minNormalCosine = cos(radians(10.0f));
	float suitableSamplesCount = 0.0f;

	ivec3 gridIndex = ivec3(worldPos.xyz - probesBuffer.probes[0][0][0].position.xyz); // TODO divide by grid cell size
//	gridIndex = clamp(gridIndex, ivec3(0,0,0), ivec3(31,15,31));

	vec2 probeImageSize = vec2(textureSize( sampler2D( probesImage, repeatLinearSampler ), 0 ));
	vec2 depthImageSize = vec2(textureSize( sampler2D( probesDepthImage, repeatLinearSampler ), 0 ));
	vec3 accumulatedGI = vec3(0.0f, 0.0f, 0.0f);

	// trilinear interpolation values along axes
	DDGIProbe baseProbe = probesBuffer.probes[gridIndex.x][gridIndex.y][gridIndex.z];
    vec3 alpha = clamp((worldPos.xyz - baseProbe.position.xyz) / vec3(1.0f), vec3(0.0f), vec3(1.0f));

	for (uint idx = 0; idx < 8; ++idx)
	{
		if (max(gridIndex.x, gridIndex.z) > 30 || gridIndex.y > 14)
		continue;
		if (min(gridIndex.x, min(gridIndex.y, gridIndex.z)) < 0)
		continue;

		ivec3 probeOffset = ivec3(idx, idx >> 1, idx >> 2) & ivec3(1);
		ivec3 probeIndex = gridIndex + probeOffset;
		DDGIProbe probe = probesBuffer.probes[probeIndex.x][probeIndex.y][probeIndex.z];

//		if (probe.position.y > 0.1) continue;

		vec3 pixelToProbe = probe.position.xyz - worldPos.xyz;
		if (length(pixelToProbe) < 0.001)
		{
//			pixelToProbe = normal;
		}
		vec3 pixelToProbeDirection = normalize(pixelToProbe);

		//float weight = (dot(normal, pixelToProbeDirection) + 1.0f) * 0.5f;//max(dot(normal, pixelToProbeDirection), 0.005f);//
		// another smoothing bias
		//weight = sqrt(weight)/*weight*/ + 0.2;
		vec3 trilinear = mix(vec3(1.0) - alpha, alpha, probeOffset);
        float weight = trilinear.x * trilinear.y * trilinear.z + 0.001; // another bias

		// texture positions for radiocity and depth
		uvec2 texturePos = uvec2(probe.texturePosition >> 16, probe.texturePosition & 0xffffu);
		uvec2 depthPos = uvec2(probe.depthPosition >> 16, probe.depthPosition & 0xffffu);

		vec2 startUV = (vec2(texturePos) + vec2(1.0f)) / probeImageSize;
		vec2 oemUV = (DirectionToOctahedronUV(normal) * vec2(6.0, 6.0) + vec2(texturePos) + vec2(1.0f)) / probeImageSize;
		vec2 depthStartUV = (vec2(depthPos) + vec2(1.0f)) / depthImageSize;
		vec2 depthOemUV = DirectionToOctahedronUV(-pixelToProbeDirection) * (vec2(16.0f) / depthImageSize);
		
		vec2 uvOffset = 1.0f / probeImageSize;
		vec2 uvDepthOffset = 1.0f / depthImageSize;

		vec4 light = vec4(0.0f);
		vec2 probeDepth = vec2(0.0);

		for (int pixelX = -1; pixelX < 2; ++pixelX)
		{
			for (int pixelY = -1; pixelY < 2; ++pixelY)
			{
				vec2 offset = uvOffset * vec2(pixelX, pixelY);
				light += texture( sampler2D( probesImage, borderBlackLinearSampler ), /*startUV +*/ oemUV + offset);
				probeDepth += texture( sampler2D( probesDepthImage, borderBlackLinearSampler ), depthStartUV + depthOemUV + offset).xy;
			}
		}
		light /= 9.0f;
		probeDepth /= 9.0f;
		
		// probe depth x is mean and y is mean squared, using Chebyshev inequality
		probeDepth = texture( sampler2D( probesDepthImage, borderBlackLinearSampler ), depthStartUV + depthOemUV).xy;
		float mean = probeDepth.x;
		float mean2 = probeDepth.y;
		float r = length(pixelToProbe);// + 0.1;
		if (r > mean)// && mean > 0.0f)
		{
//			continue;
			float variance = abs((mean*mean) - mean2) + 0.01;
			weight *= variance / (variance + pow(r - mean, 2.0));
			//weight *= 0.5; 
		}

		if (weight < 0.1)
		{
//			weight *= (weight*weight) / (0.1*0.1);
		}

		//if (light.a > length(pixelToProbe))
		{
			accumulatedGI += light.rgb * weight;
		}
		//accumulatedGI = vec3(oemUV, 0.0);
	}

	vec3 albedo = texture( sampler2D( albedoTex, repeatLinearSampler ), uv ).xyz;
	vec3 directLight = texture( sampler2D( frameDirectLight, repeatLinearSampler ), uv ).xyz;
	outColor = vec4(directLight + accumulatedGI * albedo, 1.0f);
	//outColor = vec4(accumulatedGI, 1.0f);
	//outColor = vec4(normal, 1.0f);
	//outColor = vec4(DirectionToOctahedronUV(normal), 0.0f, 1.0f);
	//outColor = vec4(vec3(gridIndex) / 31.0f, 1.0f);
	//outColor = vec4(baseProbe.position.xyz / 32.0f, 1.0f);
}
