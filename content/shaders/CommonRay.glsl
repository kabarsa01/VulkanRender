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
	uint temporalCounter;
};

vec2 DirectionToOctahedronUV(in vec3 direction)
{
	float zSign = sign(direction.z);
	float anglePhi = degrees(acos(dot(normalize(direction.zx), vec2(zSign, 0.0f))));
	float angleTheta = degrees(acos(dot(direction, vec3(0.0f, 1.0f, 0.0f))));

	vec2 ecuatorVector = vec2(-0.5f * zSign, 0.5f * sign(direction.x));
	vec2 startCorner = vec2(0.5f + 0.5f * zSign, 0.5f);

	vec2 middlePoint = startCorner + (ecuatorVector * (anglePhi / 90.0f));

	if (angleTheta <= 90.0f)
	{
		vec2 uv = vec2(0.5f, 0.5f) + (middlePoint - vec2(0.5f, 0.5f)) * (angleTheta / 90.0f);
		return vec2(uv.x, 1.0f - uv.y);
	}

	vec2 southPole = vec2(0.5f + 0.5f * zSign, 0.5f + 0.5f * sign(direction.x));
	vec2 uv = middlePoint + (southPole - middlePoint) * ((angleTheta - 90.0f) / 90.0f);
	return vec2(uv.x, 1.0f - uv.y);
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
