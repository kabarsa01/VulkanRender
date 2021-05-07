#ifndef _MATH_3D_H_
#define _MATH_3D_H_

#include "glm/glm.hpp"
#include "scene/camera/CameraComponent.h"

namespace CGE
{

	struct Plane
	{
		float A, B, C, D;
	};

	struct PlaneNorm
	{
		glm::vec3 normal;
		glm::vec3 point;
	};

	struct Line
	{
		glm::vec3 start;
		glm::vec3 end;
	};

	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 dir;
	};

	struct RayNorm
	{
		glm::vec3 origin;
		glm::vec3 dirNorm;
		float length;
	};

	struct Sphere
	{
		glm::vec3 center;
		float radius;
	};

	struct Frustum
	{
		// 5 planes forming frustum pyramid
		PlaneNorm planes[5];
		// 4 points forming far plane quad
		// together with origin point they form
		// a pyramid of our frustum without near plane
		glm::vec3 points[4];
		glm::vec3 origin;
		// normalized forward direction from origin point
		glm::vec3 forwardDir;
		// near and far along forward direction vector
		float nearPlane;
		float farPlane;
	};

	struct AABB
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	Frustum CreateFrustum(CameraComponentPtr cameraComp);

	bool FComp(float f1, float f2, float epsilon = 0.0001f);

	bool IsPointInFrustum(const Frustum& f, const glm::vec3& p);
	bool IsPointInAABB(const AABB& aabb, const glm::vec3& p);

	bool RayIntersect(const AABB& aabb, const Ray& ray);
	bool FrustumIntersectSlow(const Frustum& f, const AABB& aabb);

}

#endif
