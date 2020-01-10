#pragma once

#include "scene/SceneObjectBase.h"
#include "LightComponent.h"
#include <memory>

class LightObject : public SceneObjectBase
{
public:
	LightObject();
	virtual ~LightObject();

	std::shared_ptr<LightComponent> GetLightComponent();
protected:
	std::shared_ptr<LightComponent> LightComp;

	virtual void IntializeComponents() override;
};

typedef std::shared_ptr<LightObject> LightObjectPtr;

