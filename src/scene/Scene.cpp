#include "scene/Scene.h"
#include "scene/SceneObjectBase.h"
#include "scene/SceneObjectComponent.h"

#include "core/TimeManager.h"

Scene::Scene()
	: ObjectBase()
{
}

Scene::~Scene()
{
}

void Scene::OnInitialize()
{
	ObjectBase::OnInitialize();
}

void Scene::RegisterSceneObject(SceneObjectBasePtr InSceneObject)
{
	SceneObjectsSet.insert(InSceneObject);
	SceneObjectsMap[InSceneObject->GetClass().GetName()].insert(InSceneObject);
}

void Scene::RemoveSceneObject(SceneObjectBasePtr InSceneObject)
{
	SceneObjectsSet.erase(InSceneObject);
	SceneObjectsMap[InSceneObject->GetClass().GetName()].erase(InSceneObject);
}

void Scene::RegisterSceneObjectComponent(SceneObjectComponentPtr InSceneObjectComponent)
{
	SceneObjectComponents[InSceneObjectComponent->GetClass().GetName()].insert(InSceneObjectComponent);
}

void Scene::RemoveSceneObjectComponent(SceneObjectComponentPtr InSceneObjectComponent)
{
	SceneObjectComponents[InSceneObjectComponent->GetClass().GetName()].erase(InSceneObjectComponent);
}

void Scene::PerFrameUpdate()
{
	float DeltaTime = TimeManager::GetInstance()->GetDeltaTime();
	for (SceneObjectBasePtr SceneObject : SceneObjectsSet)
	{
		if (SceneObject->IsTickEnabled)
		{
			SceneObject->Tick(DeltaTime);
			SceneObject->TickComponents(DeltaTime);
		}
	}
}
