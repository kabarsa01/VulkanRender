#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#include "scene/SceneObjectComponent.h"
#include <memory>

class CameraComponent : public SceneObjectComponent
{
public:
	CameraComponent(std::shared_ptr<SceneObjectBase> inParent);
	virtual ~CameraComponent();

	//---------------------------------------------------------------------------
	// Accessors
	//---------------------------------------------------------------------------
	float GetFov() const { return fov; }
	void SetFov(float inFov) { this->fov = inFov; }

	float GetAspectRatio() const { return aspectRatio; }
	void SetAspectRatio(float inAspectRatio) { this->aspectRatio = inAspectRatio; }

	float GetNearPlane() const { return nearPlane; }
	void SetNearPlane(float inNearPlane) { this->nearPlane = inNearPlane; }

	float GetFarPlane() const { return farPlane; }
	void SetFarPlane(float inFarPlane) { this->farPlane = inFarPlane; }
	//---------------------------------------------------------------------------
	// 
	//---------------------------------------------------------------------------
	glm::mat4 CalculateViewMatrix() const;
	glm::mat4 CalculateProjectionMatrix() const;
protected:
	float fov = 60.0f;
	float aspectRatio = 16.0f / 9.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
};

typedef std::shared_ptr<CameraComponent> CameraComponentPtr;



