#include "scene/Scene.h"
#include "scene/SceneObjectBase.h"
#include "scene/SceneObjectComponent.h"

#include "core/TimeManager.h"
#include "camera/CameraObject.h"
#include "mesh/MeshObject.h"
#include "import/MeshImporter.h"

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
	cameraObj->transform.SetLocation({ 0.0f, 0.0f, 35.0f });
	cameraObj->transform.SetRotation({ 0.0f, 180.0f, 0.0f });
	cameraObj->GetCameraComponent()->SetFov(110.0f);

	{
		MeshImporter Importer;
		//Importer.Import("./content/root/Aset_wood_root_M_rkswd_LOD0.FBX");
		Importer.Import("./content/meshes/gun/Cerberus_LP.FBX");
		for (unsigned int MeshIndex = 0; MeshIndex < Importer.GetMeshes().size(); MeshIndex++)
		{
			MeshObjectPtr MO = ObjectBase::NewObject<MeshObject>();
			MO->GetMeshComponent()->meshData = Importer.GetMeshes()[MeshIndex];
			MO->transform.SetLocation({ 0.0f, 0.0f, 0.0f });
			MO->transform.SetScale({ 0.3f, 0.3f, 0.3f });
//			MO->GetMeshComponent()->meshData->CreateBuffer();
		}
	}
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
		if (SceneObject->isTickEnabled)
		{
			SceneObject->Tick(DeltaTime);
			SceneObject->TickComponents(DeltaTime);
		}
	}
}
