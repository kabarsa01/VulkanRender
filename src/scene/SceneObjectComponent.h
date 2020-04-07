#pragma once
#include "core\ObjectBase.h"
#include <memory>

class SceneObjectBase;

class SceneObjectComponent : public ObjectBase
{
public:
	bool isTickEnabled = true;

	SceneObjectComponent(std::shared_ptr<SceneObjectBase> inParent);
	virtual ~SceneObjectComponent();

	virtual void OnInitialize() override;
	std::shared_ptr<SceneObjectBase> GetParent();

	virtual void TickComponent(float inDeltaTime);
protected:
	std::shared_ptr<SceneObjectBase> parent;

	bool Register();
private:
	SceneObjectComponent();
};

typedef std::shared_ptr<SceneObjectComponent> SceneObjectComponentPtr;


