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

	void Init();
	
	void RegisterSceneObject(SceneObjectBasePtr inSceneObject);
	void RemoveSceneObject(SceneObjectBasePtr inSceneObject);

	void RegisterSceneObjectComponent(SceneObjectComponentPtr inSceneObjectComponent);
	void RemoveSceneObjectComponent(SceneObjectComponentPtr inSceneObjectComponent);

	void PrepareObjectsLists();

	void PerFrameUpdate();

	template<class T>
	std::set<SceneObjectComponentPtr> GetSceneComponents();
	template<class T>
	std::vector<std::shared_ptr<T>> GetSceneComponentsCast();
	template<class T>
	std::shared_ptr<T> GetSceneComponent();
protected:
	std::set<SceneObjectBasePtr> sceneObjectsSet;
	std::map<HashString, std::set<SceneObjectBasePtr>> sceneObjectsMap;
	std::map<HashString, std::set<SceneObjectComponentPtr>> sceneObjectComponents;

	std::vector<HashString> componentTypesOrder;
	std::map<HashString, std::vector<SceneObjectComponentPtr>> componentsVectors;
};

typedef std::shared_ptr<Scene> ScenePtr;

//=============================================================================================================
// TEMPLATED DEFINITIONS
//=============================================================================================================

template<class T>
inline std::set<SceneObjectComponentPtr> Scene::GetSceneComponents()
{
	HashString Key = Class::Get<T>().GetName();
	return sceneObjectComponents[Key];
}

//-------------------------------------------------------------------------------------------

template<class T>
inline std::vector<std::shared_ptr<T>> Scene::GetSceneComponentsCast()
{
	HashString Key = Class::Get<T>().GetName();
	std::vector<std::shared_ptr<T>> Components;
	for (SceneObjectComponentPtr Comp : sceneObjectComponents[Key])
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
	std::set<SceneObjectComponentPtr>& Components = sceneObjectComponents[Key];
	if (Components.size() > 0)
	{
		return ObjectBase::Cast<T, SceneObjectComponent>( * Components.begin() );
	}
	return nullptr;
}
