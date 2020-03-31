#include "scene/Scene.h"
#include "scene/SceneObjectBase.h"
#include "scene/SceneObjectComponent.h"

#include "core/TimeManager.h"
#include "camera/CameraObject.h"
#include "mesh/MeshObject.h"

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

void Scene::Init()
{
	// hardcoding dirty sample scene 
	CameraObjectPtr cameraObj = ObjectBase::NewObject<CameraObject>();
	cameraObj->Transform.SetLocation({ 0.0f, 0.0f, 3.0f });
	cameraObj->Transform.SetRotation({ 0.0f, 180.0f, 0.0f });
	MeshObjectPtr meshObj = ObjectBase::NewObject<MeshObject>();
	meshObj->GetMeshComponent()->SetMeshData(MeshData::FullscreenQuad());
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
