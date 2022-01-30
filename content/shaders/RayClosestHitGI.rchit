#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#include "CommonRay.glsl"

layout(location = 0) rayPayloadInEXT HitPayloadGI payload;

void main()
{
	payload.worldPos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	payload.hitT = gl_HitTEXT;
	payload.instanceCustomId = gl_InstanceCustomIndexEXT;
	//payload.color = vec3(1.0f, 1.0f, 1.0f);
}
