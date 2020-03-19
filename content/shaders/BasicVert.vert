#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform MVPBuffer
{
	mat4 model;
	mat4 view;
	mat4 proj;
} mvpBuffer;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = mvpBuffer.proj * mvpBuffer.view * mvpBuffer.model * vec4(inPos, 1.0);
    fragColor = inNormal;
}
