#pragma once

#include "scene/SceneObjectBase.h"
#include "CameraComponent.h"
#include <memory>

class CameraObject : public SceneObjectBase
{
public:
	CameraObject();
	virtual ~CameraObject();

	std::shared_ptr<CameraComponent> GetCameraComponent();
protected:
	std::shared_ptr<CameraComponent> CameraComp;

	virtual void IntializeComponents() override;
};

typedef std::shared_ptr<CameraObject> CameraObjectPtr;

