#pragma once

#include "scene/SceneObjectBase.h"
#include "scene/mesh/MeshComponent.h"
#include <memory>

class MeshObject : public SceneObjectBase
{
public:
	MeshObject();
	virtual ~MeshObject();

	virtual void OnInitialize() override;

	std::shared_ptr<MeshComponent> GetMeshComponent();
protected:
	std::shared_ptr<MeshComponent> MeshComp;

	virtual void IntializeComponents() override;
};

typedef std::shared_ptr<MeshObject> MeshObjectPtr;
