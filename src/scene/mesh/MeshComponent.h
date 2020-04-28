#pragma once

#include "scene/SceneObjectComponent.h"
#include <vector>
#include "data/MeshData.h"
#include "data/Material.h"

class MeshComponent : public SceneObjectComponent
{
public:
	MeshDataPtr meshData;
	MaterialPtr material;
	bool castShadows = true;

	MeshComponent(std::shared_ptr<SceneObjectBase> inParent);
	virtual ~MeshComponent();

	void SetMeshData(MeshDataPtr inMeshData);
	void SetMaterial(MaterialPtr inMaterial);

	virtual void OnInitialize() override;
protected:
};

typedef std::shared_ptr<MeshComponent> MeshComponentPtr;

