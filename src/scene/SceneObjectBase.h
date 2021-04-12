#pragma once

#include "scene/Transform.h"
#include <vector>
#include <memory>

#include "core/ObjectBase.h"
#include "core/Class.h"

namespace CGE
{
	class SceneObjectComponent;
	typedef std::shared_ptr<SceneObjectComponent> SceneObjectComponentPtr;
	
	class SceneObjectBase : public ObjectBase, public ClassType<SceneObjectBase>
	{
	public:
		// fields
		Transform transform;
		bool isTickEnabled = true;
	
		// methods
		SceneObjectBase();
		virtual ~SceneObjectBase();
	
		virtual void OnInitialize() override;
		bool RegisterComponent(std::shared_ptr<SceneObjectComponent> inComponent);
		virtual void Tick(float inDeltaTime);
		void TickComponents(float inDeltaTime);
	
		template<class T>
		std::shared_ptr<T> GetComponentByType();
		template<class T>
		SceneObjectComponentPtr GetComponent();
		std::vector<SceneObjectComponentPtr> GetComponents() const;
	
		virtual void OnDestroy() override;
	protected:
		// components container
		std::vector<SceneObjectComponentPtr> components;
	
		virtual void IntializeComponents();
	};
	
	typedef std::shared_ptr<SceneObjectBase> SceneObjectBasePtr;
	
	//---------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------
	
	template<class T>
	inline std::shared_ptr<T> SceneObjectBase::GetComponentByType()
	{
		return ObjectBase::Cast<T, SceneObjectComponent>(GetComponent<T>());
	}
	
	//---------------------------------------------------------------------------------------------------
	
	template<class T>
	inline SceneObjectComponentPtr SceneObjectBase::GetComponent()
	{
		Class<T>* searchedClass = Class<T>::Get();
		for (SceneObjectComponentPtr component : components)
		{
			if (searchedClass->Equals(component->GetClass()))
			{
				return component;
			}
		}
	
		return SceneObjectComponentPtr{ nullptr };
	}
}

