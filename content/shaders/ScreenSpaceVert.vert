#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "CommonFrameData.glsl"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 uv;

void main() {
    gl_Position = vec4(inPos, 1.0);
	normal = inNormal;
	uv = inUV;
}