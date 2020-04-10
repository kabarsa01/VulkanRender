#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

class Transform
{
public:
	Transform();
	virtual ~Transform();

	const glm::vec3 & GetLocation() const { return location; }
	void SetLocation(const glm::vec3 & inLocation);
	void AddLocation(const glm::vec3 & inLocation);

	const glm::vec3 & GetRotation() const { return rotation; }
	void SetRotation(const glm::vec3 & inRotation);
	void AddRotation(const glm::vec3 & inRotation);

	const glm::vec3 & GetScale() const { return scale; }
	void SetScale(const glm::vec3 & inScale);
	void AddScale(const glm::vec3 & inScale);

	void MarkDirty();
	glm::mat4 GetMatrix() const;
	glm::mat4& GetMatrix();

	glm::mat4 CalculateRotationMatrix() const;
	glm::mat4 CalculateViewMatrix() const;
	glm::mat4 CalculateMatrix() const;

	glm::vec3 GetForwardVector() const;
protected:
	glm::vec3 location;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::mat4 matrix;
	bool isDirty;
};

