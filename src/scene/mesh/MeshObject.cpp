#include "MeshObject.h"

MeshObject::MeshObject()
{
}


MeshObject::~MeshObject()
{
}

void MeshObject::OnInitialize()
{
	SceneObjectBase::OnInitialize();
}

std::shared_ptr<MeshComponent> MeshObject::GetMeshComponent()
{
	return MeshComp;
}

void MeshObject::IntializeComponents()
{
	SceneObjectBase::IntializeComponents();

	MeshComp = ObjectBase::NewObject<MeshComponent, std::shared_ptr<SceneObjectBase>>(get_shared_from_this<SceneObjectBase>());
}
