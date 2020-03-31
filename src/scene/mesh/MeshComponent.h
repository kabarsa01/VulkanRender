#pragma once

#include "scene/SceneObjectComponent.h"
#include <vector>
#include "data/MeshData.h"

class MeshComponent : public SceneObjectComponent
{
public:
	MeshDataPtr MeshData;
//	MaterialPtr Material;
	bool CastShadows = true;

	MeshComponent(std::shared_ptr<SceneObjectBase> Parent);
	virtual ~MeshComponent();

	void SetMeshData(MeshDataPtr InMeshData);
	virtual void OnInitialize() override;
protected:
};

typedef std::shared_ptr<MeshComponent> MeshComponentPtr;

