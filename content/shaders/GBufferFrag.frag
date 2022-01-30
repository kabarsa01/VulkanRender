#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "CommonFrameData.glsl"

layout(early_fragment_tests) in;

layout(set = 1, binding = 1) uniform texture2D albedo;
layout(set = 1, binding = 2) uniform texture2D normal;

layout(location = 0) in FragmentInput {
	vec3 worldPos;
	vec3 worldNormal;
	vec2 uv;
    vec4 framePosProjected;
	vec4 prevFramePosProjected;
	mat3x3 TBN;
} fragInput;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec2 outVelocity;

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
	vec3 tangentNormal = texture( sampler2D( normal, repeatLinearSampler ), fragInput.uv ).xyz * 2.0 - 1.0;
	outNormal = vec4(fragInput.TBN * tangentNormal, 1.0);

    vec2 pos = (fragInput.framePosProjected.xy / fragInput.framePosProjected.w) * 0.5f + 0.5f;
    vec2 prevPos = (fragInput.prevFramePosProjected.xy / fragInput.prevFramePosProjected.w) * 0.5f + 0.5f;
    outVelocity = (prevPos - pos) * 10.0f;
}




