#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

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

layout(set = 1, binding = 1) uniform texture2D albedo;
layout(set = 1, binding = 2) uniform texture2D normal;

layout(location = 0) in FragmentInput {
	vec3 worldPos;
	vec3 worldNormal;
	vec2 uv;
} fragInput;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outTangent;
layout(location = 3) out vec4 outBitangent;

vec3 PerturbNormal()
{
	vec3 tangentNormal = texture( sampler2D( normal, repeatLinearSampler ), fragInput.uv ).xyz * 2.0 - 1.0;

	vec3 pos_dx = dFdx(fragInput.worldPos);
    vec3 pos_dy = dFdy(fragInput.worldPos);
    vec3 tex_dx = dFdx(vec3(fragInput.uv, 0.0));
    vec3 tex_dy = dFdy(vec3(fragInput.uv, 0.0));
    vec3 t = (tex_dy.y * pos_dx - tex_dx.y * pos_dy) / (tex_dx.x * tex_dy.y - tex_dy.x * tex_dx.y);

    vec3 ng = normalize(fragInput.worldNormal);

    t = normalize(t - ng * dot(ng, t));
    vec3 b = normalize(cross(ng, t));
    mat3 tbn = mat3(t, b, ng);
    
    return normalize(tbn * tangentNormal);
}

void main() {
    outAlbedo = texture( sampler2D( albedo, repeatLinearSampler ), fragInput.uv );
	outNormal = vec4(PerturbNormal(), 1.0);
}




