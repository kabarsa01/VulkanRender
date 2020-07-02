#version 450
#extension GL_ARB_separate_shader_objects : enable

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

layout(set = 1, binding = 1) uniform texture2D screenImage;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 color = texture( sampler2D( screenImage, repeatLinearSampler ), uv );
	// tonemap
    color.xyz = color.xyz / (color.xyz + vec3(1.0, 1.0, 1.0) );
	// gamma correct
	outColor = pow(color, vec4(1.0/2.2));
}
