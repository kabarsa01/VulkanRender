//#version 460
//#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query : require
//#extension GL_EXT_nonuniform_qualifier : enable
//#extension GL_EXT_scalar_block_layout : enable
//#extension GL_GOOGLE_include_directive : enable

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