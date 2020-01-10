#include "SkyboxObject.h"

SkyboxObject::SkyboxObject()
{
}


SkyboxObject::~SkyboxObject()
{
}

void SkyboxObject::OnInitialize()
{
	SceneObjectBase::OnInitialize();
}

std::shared_ptr<SkyboxComponent> SkyboxObject::GetMeshComponent()
{
	return SkyComp;
}

void SkyboxObject::IntializeComponents()
{
	SceneObjectBase::IntializeComponents();

	SkyComp = ObjectBase::NewObject<SkyboxComponent, std::shared_ptr<SceneObjectBase>>(get_shared_from_this<SceneObjectBase>());
}

