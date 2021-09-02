#ifndef _COMMON_DEPTH_GLSL_
#define _COMMON_DEPTH_GLSL_

float LinearizeDepth(float depth, float near, float far)
{
	return near * far / (far + depth * (near - far));
}

#endif