#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler repeatLinearSampler;
layout(set = 0, binding = 1) uniform sampler repeatMirrorLinearSampler;
layout(set = 0, binding = 2) uniform sampler borderBlackLinearSampler;
layout(set = 0, binding = 3) uniform sampler borderWhiteLinearSampler;
layout(set = 0, binding = 4) uniform ShaderGlobalData
{
	mat4 view;
	mat4 proj;
	float time;
	float deltaTime;
	float cameraNear;
	float cameraFar;
} globalData;

layout(set = 1, binding = 1) uniform texture2D albedoTex;
layout(set = 1, binding = 2) uniform texture2D normalsTex;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outScreenColor;

void main() {
	// dummy
    outScreenColor = texture( sampler2D( albedoTex, repeatLinearSampler ), uv );
}
