#include "scene/camera/CameraComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "scene/SceneObjectBase.h"
#include "scene/SceneObjectComponent.h"

CameraComponent::CameraComponent(std::shared_ptr<SceneObjectBase> InParent)
	: SceneObjectComponent(InParent)
	, FOV(60.0f)
	, AspectRatio(16.0f / 9.0f)
	, NearPlane(0.1f)
	, FarPlane(100.0f)
{

}

CameraComponent::~CameraComponent()
{
}

glm::mat4 CameraComponent::CalculateViewMatrix() const
{
	glm::vec3 Eye = Parent->Transform.GetLocation();
	glm::vec3 Direction = Parent->Transform.GetForwardVector();
	return glm::lookAt(Eye, Eye + Direction, glm::vec3{ 0.0f, 1.0f, 0.0f });
}

glm::mat4 CameraComponent::CalculateProjectionMatrix() const
{
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(FOV), AspectRatio, NearPlane, FarPlane);
	return projection;
}
