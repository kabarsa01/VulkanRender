#pragma once

#include <map>
#include <set>
#include <vector>
#include <memory>

#include "core/ObjectBase.h"
#include "core/Class.h"
#include "common/HashString.h"

class SceneObjectBase;
typedef std::shared_ptr<SceneObjectBase> SceneObjectBasePtr;
class SceneObjectComponent;
typedef std::shared_ptr<SceneObjectComponent> SceneObjectComponentPtr;

class Scene : public ObjectBase
{
public:
	Scene();
	virtual ~Scene();

	virtual void OnInitialize() override;
	
	void RegisterSceneObject(SceneObjectBasePtr InSceneObject);
	void RemoveSceneObject(SceneObjectBasePtr InSceneObject);

	void RegisterSceneObjectComponent(SceneObjectComponentPtr InSceneObjectComponent);
	void RemoveSceneObjectComponent(SceneObjectComponentPtr InSceneObjectComponent);

	void PerFrameUpdate();

	template<class T>
	std::set<SceneObjectComponentPtr> GetSceneComponents();
	template<class T>
	std::vector<std::shared_ptr<T>> GetSceneComponentsCast();
	template<class T>
	std::shared_ptr<T> GetSceneComponent();
protected:
	std::set<SceneObjectBasePtr> SceneObjectsSet;
	std::map<HashString, std::set<SceneObjectBasePtr>> SceneObjectsMap;
	std::map<HashString, std::set<SceneObjectComponentPtr>> SceneObjectComponents;
};

typedef std::shared_ptr<Scene> ScenePtr;

//=============================================================================================================
// TEMPLATED DEFINITIONS
//=============================================================================================================

template<class T>
inline std::set<SceneObjectComponentPtr> Scene::GetSceneComponents()
{
	HashString Key = Class::Get<T>().GetName();
	return SceneObjectComponents[Key];
}

//-------------------------------------------------------------------------------------------

template<class T>
inline std::vector<std::shared_ptr<T>> Scene::GetSceneComponentsCast()
{
	HashString Key = Class::Get<T>().GetName();
	std::vector<std::shared_ptr<T>> Components;
	for (SceneObjectComponentPtr Comp : SceneObjectComponents[Key])
	{
		Components.push_back(ObjectBase::Cast<T, SceneObjectComponent>( Comp ));
	}
	return Components;
}

//-------------------------------------------------------------------------------------------

template<class T>
inline std::shared_ptr<T> Scene::GetSceneComponent()
{
	HashString Key = Class::Get<T>().GetName();
	std::set<SceneObjectComponentPtr>& Components = SceneObjectComponents[Key];
	if (Components.size() > 0)
	{
		return ObjectBase::Cast<T, SceneObjectComponent>( * Components.begin() );
	}
	return nullptr;
}
