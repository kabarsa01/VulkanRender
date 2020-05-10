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
} globalData;

layout(set = 1, binding = 0) uniform MVPBuffer
{
	mat4 model;
	mat4 view;
	mat4 proj;
} mvpBuffer;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uv;

void main() {
    gl_Position = globalData.proj * globalData.view * mvpBuffer.model * vec4(inPos, 1.0);
    fragColor = inNormal*0.5 + 0.5;//inPos + vec3(0.5, 0.5, 0.5);
	uv = inUV;
}
