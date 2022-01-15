#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "CommonFrameData.glsl"

layout(early_fragment_tests) in;

layout(set = 1, binding = 0) uniform texture2D screenImage;
layout(set = 1, binding = 1) uniform texture2D indirectLight;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 color = texture( sampler2D( screenImage, repeatLinearSampler ), uv );
	color.xyz += texture( sampler2D( indirectLight, repeatLinearSampler ), uv ).xyz;
	// tonemap
    color.xyz = color.xyz / (color.xyz + vec3(1.0, 1.0, 1.0) );
	// gamma correct
	outColor = pow(color, vec4(1.0/2.2));
}
