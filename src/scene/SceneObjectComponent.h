#pragma once
#include "core\ObjectBase.h"
#include <memory>

class SceneObjectBase;

class SceneObjectComponent : public ObjectBase
{
public:
	bool IsTickEnabled = true;

	SceneObjectComponent(std::shared_ptr<SceneObjectBase> InParent);
	virtual ~SceneObjectComponent();

	virtual void OnInitialize() override;
	std::shared_ptr<SceneObjectBase> GetParent();

	virtual void TickComponent(float DeltaTime);
protected:
	std::shared_ptr<SceneObjectBase> Parent;

	bool Register();
private:
	SceneObjectComponent();
};

typedef std::shared_ptr<SceneObjectComponent> SceneObjectComponentPtr;


