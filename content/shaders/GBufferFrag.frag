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

layout(set = 1, binding = 1) uniform texture2D albedo;
layout(set = 1, binding = 2) uniform texture2D normal;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;

void main() {
	vec4 rgba = texture( sampler2D( albedo, repeatLinearSampler ), uv );
	//rgba *= cos(fract(globalData.time) * 3.14 * 2.0) * 0.5 + 1.0;
    outAlbedo = rgba; //vec4(fragColor, 1.0);
	outNormal = texture( sampler2D( normal, repeatLinearSampler ), uv );
}
