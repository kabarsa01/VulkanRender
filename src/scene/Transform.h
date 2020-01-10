#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

class Transform
{
public:
	Transform();
	virtual ~Transform();

	const glm::vec3 & GetLocation() const { return Location; }
	void SetLocation(const glm::vec3 & InLocation);

	const glm::vec3 & GetRotation() const { return Rotation; }
	void SetRotation(const glm::vec3 & InRotation);

	const glm::vec3 & GetScale() const { return Scale; }
	void SetScale(const glm::vec3 & InScale);

	void MarkDirty();
	glm::mat4 GetMatrix() const;
	glm::mat4& GetMatrix();

	glm::mat4 CalculateRotationMatrix() const;
	glm::mat4 CalculateViewMatrix() const;
	glm::mat4 CalculateMatrix() const;

	glm::vec3 GetForwardVector() const;
protected:
	glm::vec3 Location;
	glm::vec3 Rotation;
	glm::vec3 Scale;
	glm::mat4 Matrix;
	bool IsDirty;
};

