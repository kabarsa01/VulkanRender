#include "scene/Transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

Transform::Transform()
	: Location()
	, Rotation()
	, Scale(1.0f)
	, Matrix(1.0f)
	, IsDirty(true)
{
}

Transform::~Transform()
{
}

void Transform::SetLocation(const glm::vec3 & InLocation)
{
	this->Location = InLocation;
	MarkDirty();
}

void Transform::SetRotation(const glm::vec3 & InRotation)
{
	this->Rotation = InRotation;
	MarkDirty();
}

void Transform::SetScale(const glm::vec3 & InScale)
{
	this->Scale = InScale;
	MarkDirty();
}

void Transform::MarkDirty()
{
	IsDirty = true;
}

glm::mat4 Transform::GetMatrix() const
{
	if (IsDirty)
	{
		return CalculateMatrix();
	}
	return Matrix;
}

glm::mat4& Transform::GetMatrix()
{
	if (IsDirty)
	{
		Matrix = CalculateMatrix();
		IsDirty = false;
	}
	return Matrix;
}

glm::mat4 Transform::CalculateRotationMatrix() const
{
	glm::mat4 Mat(1.0f);
	Mat = glm::rotate(Mat, glm::radians(Rotation.z), glm::vec3(0.0f, 0.0, 1.0f));
	Mat = glm::rotate(Mat, glm::radians(Rotation.y), glm::vec3(0.0f, 1.0, 0.0f));
	Mat = glm::rotate(Mat, glm::radians(Rotation.x), glm::vec3(1.0f, 0.0, 0.0f));

	return Mat;
}

glm::mat4 Transform::CalculateViewMatrix() const
{
	glm::vec3 Direction = GetForwardVector();
	return glm::lookAt(Location, Location + Direction, glm::vec3{ 0.0f, 1.0f, 0.0f });
}

glm::mat4 Transform::CalculateMatrix() const
{
	glm::mat4 Mat(1.0f);
	Mat = glm::translate(Mat, Location);
	Mat = glm::rotate(Mat, glm::radians(Rotation.z), glm::vec3(0.0f, 0.0, 1.0f));
	Mat = glm::rotate(Mat, glm::radians(Rotation.y), glm::vec3(0.0f, 1.0, 0.0f));
	Mat = glm::rotate(Mat, glm::radians(Rotation.x), glm::vec3(1.0f, 0.0, 0.0f));
	Mat = glm::scale(Mat, Scale);

	return Mat;
}

glm::vec3 Transform::GetForwardVector() const
{
	glm::vec4 Forward = { 0.0f, 0.0f, 1.0f, 0.0f };
	Forward = CalculateRotationMatrix() * Forward;
	return glm::vec3(Forward);
}


