#pragma once

#include "scene/SceneObjectBase.h"
#include "scene/misc/SkyboxComponent.h"
#include <memory>

class SkyboxObject : public SceneObjectBase
{
public:
	SkyboxObject();
	virtual ~SkyboxObject();

	virtual void OnInitialize() override;

	std::shared_ptr<SkyboxComponent> GetMeshComponent();
protected:
	std::shared_ptr<SkyboxComponent> SkyComp;

	virtual void IntializeComponents() override;
};

typedef std::shared_ptr<SkyboxObject> SkyboxObjectPtr;
