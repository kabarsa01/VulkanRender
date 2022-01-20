#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "CommonFrameData.glsl"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out FragmentInput {
	vec3 worldPos;
	vec3 worldNormal;
	vec2 uv;
	vec2 framePosProjected;
	vec2 prevFramePosProjected;
	mat3x3 TBN;
} fragInput;

void main() {
	mat4 modelMatrix = globalTransformData.modelToWorld[pushConst.transformIndexOffset + gl_InstanceIndex];
	mat4 previousModelMatrix = globalPreviousTransformData.modelToWorld[pushConst.transformIndexOffset + gl_InstanceIndex];

	fragInput.worldPos = (modelMatrix * vec4(inPos, 1.0)).xyz;
	fragInput.uv = inUV;
	mat3 normalTransformMatrix = transpose(inverse(mat3(modelMatrix)));
	fragInput.worldNormal = normalize( normalTransformMatrix * inNormal );

	vec3 T = normalize(modelMatrix * vec4(inTangent, 0.0)).xyz;
	vec3 B = normalize(modelMatrix * vec4(inBitangent, 0.0)).xyz;
	vec3 N = normalize(modelMatrix * vec4(inNormal, 0.0)).xyz;
	fragInput.TBN = mat3x3(T, B, N);

	vec4 prevPos = globalPreviousData.viewToProj * globalPreviousData.worldToView * previousModelMatrix * vec4(inPos, 1.0);
	fragInput.prevFramePosProjected = prevPos.xy / prevPos.w;

	gl_Position = globalData.viewToProj * globalData.worldToView * modelMatrix * vec4(inPos, 1.0);
	fragInput.framePosProjected = gl_Position.xy / gl_Position.w;
}


