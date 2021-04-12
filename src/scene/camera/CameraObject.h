#pragma once

#include "scene/SceneObjectBase.h"
#include "CameraComponent.h"
#include <memory>
#include "core/Class.h"

namespace CGE
{
	class CameraObject : public SceneObjectBase, public ClassType<CameraObject>
	{
	public:
		CameraObject();
		virtual ~CameraObject();
	
		std::shared_ptr<CameraComponent> GetCameraComponent();
	protected:
		std::shared_ptr<CameraComponent> CameraComp;
	
		virtual void IntializeComponents() override;
	};
	
	typedef std::shared_ptr<CameraObject> CameraObjectPtr;
}

