#include "LightObject.h"



LightObject::LightObject()
{
}


LightObject::~LightObject()
{
}

std::shared_ptr<LightComponent> LightObject::GetLightComponent()
{
	return LightComp;
}

void LightObject::IntializeComponents()
{
	LightComp = ObjectBase::NewObject<LightComponent, std::shared_ptr<SceneObjectBase>>(get_shared_from_this<SceneObjectBase>());
}
