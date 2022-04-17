#ifndef _COMMON_RAY_GLSL_
#define _COMMON_RAY_GLSL_

//#version 460
//#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query : require
//#extension GL_EXT_nonuniform_qualifier : enable
//#extension GL_EXT_scalar_block_layout : enable
//#extension GL_GOOGLE_include_directive : enable

struct HitPayloadDebug
{
	vec3 color;
};

// ray payload for initial hemisphere rays that will hit other surfaces
struct HitPayloadGI
{
	vec3 worldPos;
	float hitT;
	uint instanceCustomId;
};

// coordinate list
struct CoordinateList
{
	vec3 coords[1000];
	uint size;
};

struct DDGIProbe
{
	vec4 position;
	uint texturePosition;
	uint depthPosition;
	uint temporalCounter;
	uint pad;
};

float altSign(float num)
{
	return num < 0.0 ? -1.0 : 1.0;
}

vec2 DirectionToOctahedronUV(in vec3 direction)
{
	float xSign = altSign(direction.x);
	float zSign = altSign(direction.z);

	vec2 phiVector = direction.zx;
	if (length(phiVector) < 0.001)
	{
		phiVector = vec2(1.0, 0.0);
	}

	float anglePhi = degrees(acos(clamp( dot( normalize(phiVector), vec2(zSign, 0.0f) )/*dot*/, -1.0, 1.0)/*clamp*/));
	float angleTheta = degrees(acos(clamp( dot( direction, vec3(0.0f, 1.0f, 0.0f) ),-1.0, 1.0 )/*clamp*/)/*acos*/);

	//return vec2(anglePhi, angleTheta) / 180.0;

	vec2 ecuatorVector = vec2(-0.5f * zSign, 0.5f * xSign);
	vec2 startCorner = vec2(0.5f + 0.5f * zSign, 0.5f);

	vec2 middlePoint = startCorner + (ecuatorVector * (anglePhi / 90.0f));

	if (angleTheta <= 90.0f)
	{
		vec2 uv = vec2(0.5f, 0.5f) + (middlePoint - vec2(0.5f, 0.5f)) * (angleTheta / 90.0f);
		return vec2(uv.x, 1.0f - uv.y);
	}

	vec2 southPole = vec2(0.5f + 0.5f * zSign, 0.5f + 0.5f * xSign);
	vec2 uv = middlePoint + (southPole - middlePoint) * ((angleTheta - 90.0f) / 90.0f);
	return vec2(uv.x, 1.0f - uv.y);
}

bool OctahedronReflectBorderPixelIndex(in ivec2 maxPixels, in ivec2 pixel, out ivec2 newPixel)
{
	bool changed = false;
	if (pixel.x <= 0 || pixel.x >= maxPixels.x - 1)
	{
		newPixel.y = maxPixels.y - pixel.y - 1;
		int signPoint = pixel.x <= 0 ? 0 : maxPixels.x;
		newPixel.x = pixel.x - int(altSign(pixel.x - signPoint));
		changed = true;
	}
	if (pixel.y <= 0 || pixel.y >= maxPixels.y - 1)
	{
		newPixel.x = maxPixels.x - pixel.x - 1;
		int signPoint = pixel.y <= 0 ? 0 : maxPixels.y;
		newPixel.y = pixel.y - int(altSign(pixel.y - signPoint));
		changed = true;
	}

	return changed;
}

bool RayQueryIsShadow(accelerationStructureEXT tlas, vec3 pos, vec3 dir, float startOffset, float dist)
{
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(
		rayQuery,
		tlas,
		gl_RayFlagsTerminateOnFirstHitEXT,
		0xffffffff, 
		pos, startOffset, dir, dist);

	while(rayQueryProceedEXT(rayQuery))
	{
		if (rayQueryGetIntersectionTypeEXT(rayQuery, false) == gl_RayQueryCandidateIntersectionTriangleEXT)
		{
			// Determine if an opaque triangle hit occurred
			rayQueryConfirmIntersectionEXT(rayQuery);
		}
	}

	if (rayQueryGetIntersectionTypeEXT(rayQuery, true) == gl_RayQueryCommittedIntersectionNoneEXT)
	{
		// Not shadow!
		return false;
	} else {
		// Shadow!
		return true;
	}
}

#endif
