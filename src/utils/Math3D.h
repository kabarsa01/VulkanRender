#ifndef _MATH_3D_H_
#define _MATH_3D_H_

#include "glm/glm.hpp"

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
		float near;
		float far;
	};

	struct AABB
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	bool FComp(float f1, float f2, float epsilon = 0.0001f)
	{
		return glm::abs(f1 - f2) <= epsilon;
	}

	bool IsPointInFrustum(const Frustum& f, const glm::vec3& p)
	{
		bool isInside = true;
		for (uint8_t index = 0; index < 5; index++)
		{
			const PlaneNorm& plane = f.planes[index];
			isInside = isInside && (glm::dot(p - plane.point, plane.normal) >= 0.0f);
		}
		return isInside;
	}

	bool IsPointInAABB(const AABB& aabb, const glm::vec3& p)
	{
		return (p.x >= aabb.min.x && p.x < aabb.max.x)
			&& (p.y >= aabb.min.y && p.y < aabb.max.y)
			&& (p.z >= aabb.min.z && p.z < aabb.max.z);
	}

	bool RayIntersect(const AABB& aabb, const Ray& ray)
	{
		// x intersection time
		const float tx1 = (aabb.min.x - ray.origin.x) / FComp(ray.dir.x, 0.0f) ? 0.00001f : ray.dir.x;
		const float tx2 = (aabb.max.x - ray.origin.x) / FComp(ray.dir.x, 0.0f) ? 0.00001f : ray.dir.x;
		// y intersection time
		const float ty1 = (aabb.min.y - ray.origin.y) / FComp(ray.dir.y, 0.0f) ? 0.00001f : ray.dir.y;
		const float ty2 = (aabb.max.y - ray.origin.y) / FComp(ray.dir.y, 0.0f) ? 0.00001f : ray.dir.y;
		// z intersection time
		const float tz1 = (aabb.min.z - ray.origin.z) / FComp(ray.dir.z, 0.0f) ? 0.00001f : ray.dir.z;
		const float tz2 = (aabb.max.z - ray.origin.z) / FComp(ray.dir.z, 0.0f) ? 0.00001f : ray.dir.z;

		// find max of min intersection times
		const float tmin = glm::max(glm::max(glm::min(tx1, tx2), glm::min(ty1, ty2)), glm::min(tz1, tz2));
		const float tmax = glm::min(glm::min(glm::max(tx1, tx2), glm::max(ty1, ty2)), glm::max(tz1, tz2));

		if (tmax < 0.0f || tmin > tmax)
		{
			return false;
		}

		return true;
	}

	bool FrustumIntersect(const Frustum& f, const AABB& aabb)
	{
		bool isPointsInAABB = IsPointInAABB(aabb, f.origin);
		for (uint8_t index = 0; index < 4; index++)
		{
			isPointsInAABB = isPointsInAABB || IsPointInAABB(aabb, f.points[index]);
		}
		if (isPointsInAABB)
		{
			return true;
		}

		bool isPointsInFrustum = false;
		// xmin points frustum check
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.min.x, aabb.min.y, aabb.min.z));
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z));
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z));
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z));
		// xmax points frustum check
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z));
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z));
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z));
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.max.x, aabb.max.y, aabb.max.z));

		if (isPointsInFrustum)
		{
			return true;
		}

		// TODO frustum edges (rays) against AABB to have fully precise test
	}

}

#endif
