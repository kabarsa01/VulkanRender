#pragma once

#include "scene/SceneObjectBase.h"
#include "scene/mesh/MeshComponent.h"
#include <memory>
#include "core/Class.h"

namespace CGE
{
	class MeshObject : public SceneObjectBase, public ClassType<MeshObject>
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
}
