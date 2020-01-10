#pragma once

#include "scene/Transform.h"
#include <vector>
#include <memory>

#include "core/ObjectBase.h"
#include "core/Class.h"

class SceneObjectComponent;
typedef std::shared_ptr<SceneObjectComponent> SceneObjectComponentPtr;

class SceneObjectBase : public ObjectBase
{
public:
	// fields
	Transform Transform;
	bool IsTickEnabled = true;

	// methods
	SceneObjectBase();
	virtual ~SceneObjectBase();

	virtual void OnInitialize() override;
	bool RegisterComponent(std::shared_ptr<SceneObjectComponent> InComponent);
	virtual void Tick(float DeltaTime);
	void TickComponents(float DeltaTime);

	template<class T>
	std::shared_ptr<T> GetComponentByType();
	template<class T>
	SceneObjectComponentPtr GetComponent();
	std::vector<SceneObjectComponentPtr> GetComponents() const;

	virtual void OnDestroy() override;
protected:
	// components container
	std::vector<SceneObjectComponentPtr> Components;

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
	const Class& SearchedClass = Class::Get<T>();
	for (SceneObjectComponentPtr Component : Components)
	{
		if (SearchedClass == Component->GetClass())
		{
			return Component;
		}
	}

	return SceneObjectComponentPtr{ nullptr };
}

