#version 450
#extension GL_ARB_separate_shader_objects : enable

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

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out FragmentInput {
	vec3 worldPos;
	vec3 worldNormal;
	vec2 uv;
} fragInput;

void main() {
	mat4 modelMatrix = globalTransformData.modelToWorld[pushConst.transformIndexOffset + gl_InstanceIndex];

	fragInput.worldPos = (modelMatrix * vec4(inPos, 1.0)).xyz;
	fragInput.uv = inUV;
	mat3 normalTransformMatrix = transpose(inverse(mat3(modelMatrix)));
	fragInput.worldNormal = normalize( normalTransformMatrix * inNormal );

	gl_Position = globalData.viewToProj * globalData.worldToView * modelMatrix * vec4(inPos, 1.0);
}


