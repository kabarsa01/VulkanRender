#pragma once

#include "scene/SceneObjectComponent.h"
#include "render/MeshData.h"
#include "render/Material.h"

#include <vector>

class MeshComponent : public SceneObjectComponent
{
public:
	MeshDataPtr MeshData;
	MaterialPtr Material;
	bool CastShadows = true;

	MeshComponent(std::shared_ptr<SceneObjectBase> Parent);
	virtual ~MeshComponent();

	void SetMeshData(MeshDataPtr InMeshData);
	virtual void OnInitialize() override;
protected:
};

typedef std::shared_ptr<MeshComponent> MeshComponentPtr;

