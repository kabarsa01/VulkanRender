#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "CommonFrameData.glsl"

layout(early_fragment_tests) in;

layout(set = 1, binding = 0) uniform texture2D frameDirectLight;
layout(set = 1, binding = 1) uniform texture2D frameGILight;
layout(set = 1, binding = 2) uniform texture2D frameDepth;
layout(set = 1, binding = 3) uniform texture2D giDepth;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 directLight = texture( sampler2D( frameDirectLight, repeatLinearSampler ), uv ).xyz;
	vec3 giLight = texture( sampler2D( frameGILight, repeatLinearSampler ), uv ).xyz;

	outColor = vec4(directLight, 1.0f);
}
