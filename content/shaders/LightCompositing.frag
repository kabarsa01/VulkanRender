#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "CommonFrameData.glsl"
#include "CommonDepth.glsl"
#include "CommonRay.glsl"
#include "CommonSampling.glsl"

layout(early_fragment_tests) in;

layout(set = 1, binding = 0) uniform texture2D frameDirectLight;
layout(set = 1, binding = 1) uniform texture2D giLight;
layout(set = 1, binding = 2) uniform texture2D depthTex;
layout(set = 1, binding = 3) uniform texture2D albedoTex;
layout(set = 1, binding = 4) uniform texture2D normalsTex;
layout(set = 1, binding = 5, rgba16f) uniform readonly image2D irradianceTexture;

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
	// irradiance texture size and min pixel coordinate for bilinear
	ivec2 irradianceTexSize = imageSize(irradianceTexture);
	ivec2 irradiancePixelCoordFloor = clamp( ivec2( floor((uv * vec2(irradianceTexSize)) - vec2(0.5,0.5)) ), ivec2(0,0), irradianceTexSize - ivec2(1,1) );

	ivec2 texSize = textureSize( sampler2D( depthTex, repeatLinearSampler ), 0 );
	//ivec2 giTexSize = textureSize( sampler2D( giLight, repeatLinearSampler ), 0 );
	vec2 giScale = vec2(8.0, 8.0);//vec2(texSize) / vec2(giTexSize);
	vec2 pixelSizeUV = vec2(1.0) / texSize;

	float depth = texture( sampler2D( depthTex, borderBlackNearestSampler ), uv ).r;
	if (depth <= 0.0f || depth >= 1.0f)
	{
		outColor = vec4(0.0f, 0.1f, 0.0f, 1.0f);
		return;
	}
	float linearDepth = LinearizeDepth(depth, globalData.cameraNear, globalData.cameraFar);
	vec4 worldPos = CalculateWorldPosition(linearDepth, globalData.cameraFov, globalData.cameraAspect, uv, globalData.worldToView);

	// pixel normal
	vec3 pixelNormal = normalize(texture( sampler2D( normalsTex, borderBlackNearestSampler ), uv ).xyz);

	//vec3 normal = normalize(vec3(0.00, -1.0, 0.00));
	//vec3 giLight = texture( sampler2D( giLight, repeatLinearSampler ), uv ).xyz;
	//ivec2 giImageSize = imageSize(frameGILight);

	float minNormalCosine = cos(radians(10.0f));
	float suitableSamplesCount = 0.0f;

	ivec3 gridIndex = ivec3(worldPos.xyz - probesBuffer.probes[0][0][0].position.xyz); // TODO divide by grid cell size
//	gridIndex = clamp(gridIndex, ivec3(0,0,0), ivec3(31,15,31));

	//vec2 probeImageSize = vec2(textureSize( sampler2D( probesImage, repeatLinearSampler ), 0 ));
	//vec2 depthImageSize = vec2(textureSize( sampler2D( probesDepthImage, repeatLinearSampler ), 0 ));
	vec3 accumulatedGI = vec3(0.0f, 0.0f, 0.0f);

	// trilinear interpolation values along axes
	//DDGIProbe baseProbe = probesBuffer.probes[gridIndex.x][gridIndex.y][gridIndex.z];
    //vec3 alpha = clamp((worldPos.xyz - baseProbe.position.xyz) / vec3(1.0f), vec3(0.0f), vec3(1.0f));

//	//-----------------------------------------------------------------------------------------------------------------------------
//	// get gi texture probe coord
//	vec2 giProbeCoord = gl_FragCoord.xy / giScale;
//	vec2 giProbeCoordMin = floor(giProbeCoord - vec2(0.5, 0.5)) + vec2(0.5, 0.5);
//
//	// first probe lerp alpha
//	vec2 lerpAlpha = giProbeCoord - giProbeCoordMin;
//
//	float probeCount = 0.01;
//	// interpolate surrounding probes
//	for (int probeIdx = 0; probeIdx < 4; probeIdx++)
//	{
//		ivec2 offset = ivec2(probeIdx / 2, probeIdx) & ivec2(1,1);
//
//		vec2 probeCenterCoord = giProbeCoordMin + vec2(offset);
//		vec2 probeCenterCoordUV = probeCenterCoord * giScale / vec2(texSize);
//
//		// current probe lerp alpha
//		vec2 lerpValue = mix(vec2(1.0, 1.0) - lerpAlpha, lerpAlpha, vec2(offset));
//
//		// probe depth and world position
//		float probeDepth = texture( sampler2D( depthTex, borderBlackNearestSampler ), probeCenterCoordUV ).r;
//		float probeLinearDepth = LinearizeDepth(probeDepth, globalData.cameraNear, globalData.cameraFar);
//		vec4 probeWorldPos = CalculateWorldPosition(probeLinearDepth, globalData.cameraFov, globalData.cameraAspect, probeCenterCoordUV, globalData.worldToView);
//		// probe normal
//		vec3 probeNormal = normalize(texture( sampler2D( normalsTex, repeatLinearSampler ), probeCenterCoordUV ).xyz);
//
//		// TODO filter/reject probes
//		vec3 probeToPixel = worldPos.xyz - probeWorldPos.xyz;
//		if (length(probeToPixel) < 0.05)
//		{
//			probeToPixel = probeNormal;
//		}
//		// depth reject
//		vec2 probeToPixelMappingUV = DirectionToOctahedronUV(probeToPixel);
//		vec2 probeToPixelUV = (floor(probeCenterCoord) * giScale + vec2(1.5) + probeToPixelMappingUV * (giScale - vec2(3.0))) / vec2(texSize);
//		float pixelDepth = texture( sampler2D( giLight, repeatLinearSampler ), probeToPixelUV).w;
//		if (pixelDepth < 0.25) pixelDepth = length(probeToPixel);
//		if (pixelDepth < length(probeToPixel))
//		{
//			//continue;
//		}
//		// normals reject
//		// TODO make proper check
//		if (dot(normalize(probeToPixel), probeNormal) < -1.0f)
//		{
//			accumulatedGI += accumulatedGI / probeCount;
//			continue;
//		}
//
//		//vec2 probeMappingUV = HemisphereDirectionToPyramidUV(probeSpaceNormal);
//		vec2 probeMappingUV = DirectionToOctahedronUV(pixelNormal);
//		vec2 probePixelUV = (floor(probeCenterCoord) * giScale + vec2(1.5) + probeMappingUV * (giScale - vec2(3.0))) / vec2(texSize);
//		vec2 probePixelUVSize = vec2(1.0) / vec2(texSize);
//
//		vec4 sampledLight = vec4(0.0);
//		for (int x = -1; x < 2; x++)
//		{
//			for (int y = -1; y < 2; y++)
//			{
//				float cosMultiplier = 0.85 + 0.15 * (1.0 - max(abs(x), abs(y)));
//				vec2 pixelOffset = vec2(x,y) * probePixelUVSize;
//				sampledLight += cosMultiplier * texture( sampler2D( giLight, repeatLinearSampler ), probePixelUV + pixelOffset);
//			}
//		}
//		sampledLight /= 9.0;
//
//		accumulatedGI += sampledLight.xyz * lerpValue.x * lerpValue.y;
//
//		probeCount += 1.0;
//	}

//	float probeCount = 0.0;
//	for (uint idx = 0; idx < 8; ++idx)
//	{
//		if (max(gridIndex.x, gridIndex.z) > 30 || gridIndex.y > 14)
//		continue;
//		if (min(gridIndex.x, min(gridIndex.y, gridIndex.z)) < 0)
//		continue;
//
//		ivec3 probeOffset = ivec3(idx, idx >> 1, idx >> 2) & ivec3(1);
//		ivec3 probeIndex = gridIndex + probeOffset;
//		DDGIProbe probe = probesBuffer.probes[probeIndex.x][probeIndex.y][probeIndex.z];
//
////		if (probe.position.y > 0.1) continue;
//
//		vec3 pixelToProbe = probe.position.xyz - worldPos.xyz;
//		if (length(pixelToProbe) < 0.005)
//		{
//			pixelToProbe = normal;
//		}
//		vec3 pixelToProbeDirection = normalize(pixelToProbe);
//
//		if (dot(normal, pixelToProbeDirection) < -0.1) continue;
//		//float weight = (dot(normal, pixelToProbeDirection) + 1.0f) * 0.5f;//max(dot(normal, pixelToProbeDirection), 0.005f);//
//		//float weight = max(dot(normal, pixelToProbeDirection), 0.0f);
//		// another smoothing bias
//		//weight = pow(weight, 0.5);// + 0.2;
//		vec3 trilinear = mix(vec3(1.0) - alpha, alpha, probeOffset);
//        float weight = trilinear.x * trilinear.y * trilinear.z + 0.001; // another bias
//
//		// texture positions for radiocity and depth
//		uvec2 texturePos = uvec2(probe.texturePosition >> 16, probe.texturePosition & 0xffffu);
//		uvec2 depthPos = uvec2(probe.depthPosition >> 16, probe.depthPosition & 0xffffu);
//
//		vec2 startUV = (vec2(texturePos) + vec2(1.0f)) / probeImageSize;
//		vec2 oemUV = (DirectionToOctahedronUV(normal) * vec2(6.0, 6.0) + vec2(texturePos) + vec2(1.0f)) / probeImageSize;
//		vec2 depthStartUV = (vec2(depthPos) + vec2(1.0f)) / depthImageSize;
//		vec2 depthOemUV = DirectionToOctahedronUV(-pixelToProbeDirection) * (vec2(16.0f) / depthImageSize);
//		
//		vec2 uvOffset = 1.0f / probeImageSize;
//		vec2 uvDepthOffset = 1.0f / depthImageSize;
//
//		vec4 light = vec4(0.0f);
//		for (int pixelX = -1; pixelX < 2; ++pixelX)
//		{
//			for (int pixelY = -1; pixelY < 2; ++pixelY)
//			{
//				vec2 offset = uvOffset * vec2(pixelX, pixelY);
//				light += texture( sampler2D( probesImage, borderBlackLinearSampler ), /*startUV +*/ oemUV + offset);
//			}
//		}
//		light /= 9.0f;
//		
//		// probe depth x is mean and y is mean squared, using Chebyshev inequality
//		vec2 probeDepth = texture( sampler2D( probesDepthImage, borderBlackLinearSampler ), depthStartUV + depthOemUV).xy;
//		float mean = probeDepth.x;
//		float mean2 = probeDepth.y;
//		float r = length(pixelToProbe);// + 0.1;
//		float weightCheb = weight;
//		if (r > mean)// && mean > 0.0f)
//		{
////			continue;
//			float variance = abs((mean*mean) - mean2) + 0.01;
//			weightCheb *= variance / (variance + pow(r - mean, 2.0));
//		}
//		//probeCount += 1.0;
//
//		if (weight < 0.25)
//		{
////			weight *= (weight*weight) / (0.25*0.25);
//		}
//
//		//if (light.a > length(pixelToProbe))
//		{
//			accumulatedGI += mix(light.rgb * weight, light.rgb * weightCheb, 0.0);
//		}
//		//accumulatedGI = vec3(oemUV, 0.0);
//	}

	vec2 irradianceCoordMin = vec2(irradiancePixelCoordFloor) + vec2(0.5, 0.5);

	// lerp alpha
	vec2 lerpAlpha = clamp((uv * irradianceTexSize) - irradianceCoordMin, vec2(0.0, 0.0), vec2(1.0, 1.0));

	float bilinearAlphas[4];
	vec4 averageIrradiance = vec4(0.0);
	float samplesCount = 0.01;
	// interpolate surrounding probes

	vec2 debugVec = vec2(0.0);

	for (int pixelIndex = 0; pixelIndex < 4; pixelIndex++)
	{
		bilinearAlphas[pixelIndex] = 0.0f;
		ivec2 offset = ivec2(pixelIndex / 2, pixelIndex) & ivec2(1,1);
		vec2 irradianceUv = (vec2(irradiancePixelCoordFloor + offset) + vec2(0.5,0.5)) / vec2(irradianceTexSize);

		float irradianceDepth = texture( sampler2D( depthTex, borderBlackNearestSampler ), irradianceUv ).r;
		float irradianceLinearDepth = LinearizeDepth(irradianceDepth, globalData.cameraNear, globalData.cameraFar);
		vec4 irradianceWorldPos = CalculateWorldPosition(irradianceLinearDepth, globalData.cameraFov, globalData.cameraAspect, irradianceUv, globalData.worldToView);
		vec3 irradianceNormal = normalize(texture( sampler2D( normalsTex, borderBlackNearestSampler ), irradianceUv ).xyz);

		// current probe lerp alpha
		vec2 lerpValue = mix(vec2(1.0, 1.0) - lerpAlpha, lerpAlpha, vec2(offset));
		float weight = dot(irradianceNormal, pixelNormal) * 0.5f + 0.5f;
		vec4 irradiance = vec4(0.0);

		bool IsTooFar = length(irradianceWorldPos.xyz - worldPos.xyz) > 5.0f;
		bool IsTooBehind = dot(normalize(worldPos.xyz - irradianceWorldPos.xyz), irradianceNormal) < -0.5f;
		if (IsTooFar || IsTooBehind)
		{
			bilinearAlphas[pixelIndex] = lerpValue.x * lerpValue.y;// * weight;
			continue;
		}
		else
		{
			irradiance = imageLoad(irradianceTexture, irradiancePixelCoordFloor + offset);
			averageIrradiance += irradiance;
		}

		accumulatedGI += irradiance.xyz * lerpValue.x * lerpValue.y * weight;

		samplesCount += 1.0f;
		//----------------------------------------
		debugVec = vec2(irradiancePixelCoordFloor + offset) * vec2(texSize) / vec2(irradianceTexSize);
	}
	averageIrradiance /= samplesCount;
	for (int idx = 0; idx < 4; idx++)
	{
		accumulatedGI += averageIrradiance.xyz * bilinearAlphas[idx];
	}
	//accumulatedGI *= 4.0 / samplesCount;

	// texSize * uv
	vec3 albedo = texture( sampler2D( albedoTex, repeatLinearSampler ), uv ).xyz;
	vec3 directLight = texture( sampler2D( frameDirectLight, repeatLinearSampler ), uv ).xyz;
	outColor = vec4(directLight + 0.01f * accumulatedGI * albedo, 1.0f);
	//outColor = vec4(accumulatedGI, 1.0f);
	//outColor = vec4(debugVec, 0.0, 1.0f);
	//outColor = vec4(lerpAlpha, 0.0, 1.0f);
	//outColor = vec4(normal, 1.0f);
	//outColor = vec4(DirectionToOctahedronUV(normal), 0.0f, 1.0f);
	//outColor = vec4(vec3(gridIndex) / 31.0f, 1.0f);
	//outColor = vec4(baseProbe.position.xyz / 32.0f, 1.0f);
}
