#pragma once

#include "scene/SceneObjectComponent.h"
#include <vector>
#include "data/MeshData.h"

class MeshComponent : public SceneObjectComponent
{
public:
	MeshDataPtr meshData;
//	MaterialPtr Material;
	bool castShadows = true;

	MeshComponent(std::shared_ptr<SceneObjectBase> inParent);
	virtual ~MeshComponent();

	void SetMeshData(MeshDataPtr inMeshData);
	virtual void OnInitialize() override;
protected:
};

typedef std::shared_ptr<MeshComponent> MeshComponentPtr;

