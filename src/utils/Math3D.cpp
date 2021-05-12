#include "utils/Math3D.h"
#include "scene/camera/CameraComponent.h"
#include "scene/SceneObjectComponent.h"
#include "scene/SceneObjectBase.h"

namespace CGE
{

	Frustum CreateFrustum(CameraComponentPtr cameraComp)
	{
		Frustum f;

		Transform& tr = cameraComp->GetParent()->transform;
		f.origin = tr.GetLocation();
		f.forwardDir = tr.GetForwardVector();
		f.farPlane = cameraComp->GetFarPlane();
		f.nearPlane = cameraComp->GetNearPlane();

		float fov = cameraComp->GetFov();
		float halfHeight = cameraComp->GetFarPlane() * glm::tan(glm::radians(fov * 0.5f));
		float halfWidth = halfHeight * cameraComp->GetAspectRatio();
		glm::vec3 halfHeightVec = tr.GetUpVector() * halfHeight;
		glm::vec3 halfWidthVec = tr.GetLeftVector() * halfWidth;
		glm::vec3 farCenter = f.origin + (f.forwardDir * f.farPlane);

		// points should go around
		f.points[0] = farCenter + halfWidthVec - halfHeightVec;
		f.points[1] = farCenter + halfWidthVec + halfHeightVec;
		f.points[2] = farCenter - halfWidthVec + halfHeightVec;
		f.points[3] = farCenter - halfWidthVec - halfHeightVec;

		// all frustum plane normals will be pointing inside the volume
		for (uint8_t idx = 0; idx < 4; idx++)
		{
			glm::vec3 first = f.points[idx] - f.origin;
			glm::vec3 second = f.points[(idx+1) % 4] - f.points[idx];
			glm::vec3 normal = glm::normalize( glm::cross(first, second) );
			if (glm::dot(normal, farCenter - f.points[idx]) <= 0)
			{
				normal *= -1.0f;
			}

			PlaneNorm plane;
			plane.normal = normal;
			plane.point = f.origin;

			f.planes[idx] = plane;
		}
		// fifth plane is a far plane
		PlaneNorm farPlane;
		farPlane.normal = f.forwardDir * -1.0f;
		farPlane.point = farCenter;
		f.planes[4] = farPlane;

		return f;
	}

	bool FComp(float f1, float f2, float epsilon /*= 0.0001f*/)
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
		const float tx1 = (aabb.min.x - ray.origin.x) / (FComp(ray.dir.x, 0.0f) ? 0.00001f : ray.dir.x);
		const float tx2 = (aabb.max.x - ray.origin.x) / (FComp(ray.dir.x, 0.0f) ? 0.00001f : ray.dir.x);
		// y intersection time
		const float ty1 = (aabb.min.y - ray.origin.y) / (FComp(ray.dir.y, 0.0f) ? 0.00001f : ray.dir.y);
		const float ty2 = (aabb.max.y - ray.origin.y) / (FComp(ray.dir.y, 0.0f) ? 0.00001f : ray.dir.y);
		// z intersection time
		const float tz1 = (aabb.min.z - ray.origin.z) / (FComp(ray.dir.z, 0.0f) ? 0.00001f : ray.dir.z);
		const float tz2 = (aabb.max.z - ray.origin.z) / (FComp(ray.dir.z, 0.0f) ? 0.00001f : ray.dir.z);

		// find max of min intersection times
		const float tmin = glm::max(glm::max(glm::min(tx1, tx2), glm::min(ty1, ty2)), glm::min(tz1, tz2));
		const float tmax = glm::min(glm::min(glm::max(tx1, tx2), glm::max(ty1, ty2)), glm::max(tz1, tz2));

		if (tmax < 0.0f || tmin > tmax)
		{
			return false;
		}

		return true;
	}

	float PlaneIntersect(const AABB& aabb, const PlaneNorm& plane)
	{
		glm::vec3 extents = (aabb.max - aabb.min) * 0.5f;
		float extentsProjected = glm::abs( glm::dot(plane.normal, extents) );

		float distanceProjected = glm::dot( plane.normal, aabb.min + extents - plane.point );
		if (glm::abs(distanceProjected) < extentsProjected)
		{
			return 0;
		}
		// return distance to plane
		return distanceProjected - (glm::sign(distanceProjected) * extentsProjected);
	}

	bool FrustumIntersectSlow(const Frustum& f, const AABB& aabb)
	{
		bool isPointsInFrustum = false;

		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, aabb.min);
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, aabb.max);
		// xmin points frustum check
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z));
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z));
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z));
		// xmax points frustum check
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z));
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z));
		isPointsInFrustum = isPointsInFrustum || IsPointInFrustum(f, glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z));
		if (isPointsInFrustum)
		{
			return true;
		}

		bool isPointsInAABB = IsPointInAABB(aabb, f.origin);
		for (uint8_t index = 0; index < 4; index++)
		{
			isPointsInAABB = isPointsInAABB || IsPointInAABB(aabb, f.points[index]);
		}
		if (isPointsInAABB)
		{
			return true;
		}

		// frustum edges (rays) against AABB to have fully precise test
		bool isRayIntersect = false;
		for (uint8_t idx = 0; idx < 4; idx++)
		{
			isRayIntersect = isRayIntersect || RayIntersect(aabb, { f.origin, f.points[idx] - f.origin });
			isRayIntersect = isRayIntersect || RayIntersect(aabb, { f.points[idx], f.points[(idx + 1) % 4] - f.points[idx] });
		}

		return isRayIntersect;
	}

	bool FrustumIntersect(const Frustum& f, const AABB& aabb)
	{
		for (uint8_t idx = 0; idx < 5; idx++)
		{
			if (PlaneIntersect(aabb, f.planes[idx]) < 0.0f)
			{
				return false;
			}
		}
		return true;
	}

}


