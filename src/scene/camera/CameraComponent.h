#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#include "scene/SceneObjectComponent.h"
#include <memory>

class CameraComponent : public SceneObjectComponent
{
public:
	CameraComponent(std::shared_ptr<SceneObjectBase> InParent);
	virtual ~CameraComponent();

	//---------------------------------------------------------------------------
	// Accessors
	//---------------------------------------------------------------------------
	float GetFOV() const { return FOV; }
	void SetFOV(float InFOV) { this->FOV = InFOV; }

	float GetAspectRatio() const { return AspectRatio; }
	void SetAspectRatio(float InAspectRatio) { this->AspectRatio = InAspectRatio; }

	float GetNearPlane() const { return NearPlane; }
	void SetNearPlane(float InNearPlane) { this->NearPlane = InNearPlane; }

	float GetFarPlane() const { return FarPlane; }
	void SetFarPlane(float InFarPlane) { this->FarPlane = InFarPlane; }
	//---------------------------------------------------------------------------
	// 
	//---------------------------------------------------------------------------
	glm::mat4 CalculateViewMatrix() const;
	glm::mat4 CalculateProjectionMatrix() const;
protected:
	float FOV = 60.0f;
	float AspectRatio = 16.0f / 9.0f;
	float NearPlane = 0.1f;
	float FarPlane = 100.0f;
};

typedef std::shared_ptr<CameraComponent> CameraComponentPtr;



