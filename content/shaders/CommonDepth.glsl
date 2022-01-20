#ifndef _COMMON_DEPTH_GLSL_
#define _COMMON_DEPTH_GLSL_

float LinearizeDepth(float depth, float near, float far)
{
	return near * far / (far + depth * (near - far));
}

vec4 CalculateWorldPosition(float linearDepth, float fov, float aspect, vec2 uv, mat4x4 worldToView)
{
	float height = 2.0 * linearDepth * tan(radians(fov * 0.5));
	float width = height * aspect;
	float pixelViewSpaceX = width * (-0.5 + uv.x);
	float pixelViewSpaceY = height * (-0.5 + uv.y);
	vec4 pixelCoordWorld = inverse(worldToView) * vec4(pixelViewSpaceX, pixelViewSpaceY, -linearDepth, 1.0);
	pixelCoordWorld /= pixelCoordWorld.w;

	return pixelCoordWorld;
}

#endif