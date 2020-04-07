#include "scene/camera/CameraComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "scene/SceneObjectBase.h"
#include "scene/SceneObjectComponent.h"

CameraComponent::CameraComponent(std::shared_ptr<SceneObjectBase> inParent)
	: SceneObjectComponent(inParent)
	, fov(60.0f)
	, aspectRatio(16.0f / 9.0f)
	, nearPlane(0.1f)
	, farPlane(100.0f)
{

}

CameraComponent::~CameraComponent()
{
}

glm::mat4 CameraComponent::CalculateViewMatrix() const
{
	glm::vec3 eye = parent->transform.GetLocation();
	glm::vec3 direction = parent->transform.GetForwardVector();
	return glm::lookAt(eye, eye + direction, glm::vec3{ 0.0f, 1.0f, 0.0f });
}

glm::mat4 CameraComponent::CalculateProjectionMatrix() const
{
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	return projection;
}
