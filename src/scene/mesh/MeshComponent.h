#pragma once

#include "scene/SceneObjectComponent.h"
#include <vector>
#include "data/MeshData.h"
#include "data/Material.h"
#include "data/RtMaterial.h"

namespace CGE
{
	class MeshComponent : public SceneObjectComponent
	{
	public:
		MeshDataPtr meshData;
		MaterialPtr material;
		RtMaterialPtr rtMaterial;
		bool castShadows = true;
	
		MeshComponent(std::shared_ptr<SceneObjectBase> inParent);
		virtual ~MeshComponent();
	
		void SetMeshData(MeshDataPtr inMeshData);
		void SetMaterial(MaterialPtr inMaterial);
		void SetRtMaterial(RtMaterialPtr inRtMaterial);
	
		virtual void OnInitialize() override;
	protected:
	};
	
	typedef std::shared_ptr<MeshComponent> MeshComponentPtr;
}

