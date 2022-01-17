#ifndef _COMMON_FRAME_DATA_GLSL_
#define _COMMON_FRAME_DATA_GLSL_

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
	mat4 previousWorldToView;
	mat4 viewToProj;
	mat4 previousViewToProj;
	vec3 cameraPos;
	vec3 previousCameraPos;
	vec3 viewVector;
	vec3 previousViewVector;
	ivec2 numClusters;
	ivec2 clusterSize;
	ivec2 halfScreenOffset;
	vec2 clusterScreenOverflow;
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

layout(set = 0, binding = 6) readonly buffer GlobalPreviousTransformData
{
	mat4 modelToWorld[];
} globalPreviousTransformData;

#endif
