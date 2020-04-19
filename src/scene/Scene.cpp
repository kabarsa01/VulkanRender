#include "scene/Scene.h"
#include "scene/SceneObjectBase.h"
#include "scene/SceneObjectComponent.h"

#include "core/TimeManager.h"
#include "camera/CameraObject.h"
#include "mesh/MeshObject.h"
#include "import/MeshImporter.h"
#include "render/TransferList.h"

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
	TransferList* TL = TransferList::GetInstance();

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
			MO->GetMeshComponent()->meshData->CreateBuffer();

			TL->PushBuffers(MO->GetMeshComponent()->meshData);
		}
	}

	MeshData::FullscreenQuad()->CreateBuffer();
	TL->PushBuffers(MeshData::FullscreenQuad());
}

void Scene::RegisterSceneObject(SceneObjectBasePtr inSceneObject)
{
	sceneObjectsSet.insert(inSceneObject);
	sceneObjectsMap[inSceneObject->GetClass().GetName()].insert(inSceneObject);
}

void Scene::RemoveSceneObject(SceneObjectBasePtr inSceneObject)
{
	sceneObjectsSet.erase(inSceneObject);
	sceneObjectsMap[inSceneObject->GetClass().GetName()].erase(inSceneObject);
}

void Scene::RegisterSceneObjectComponent(SceneObjectComponentPtr inSceneObjectComponent)
{
	sceneObjectComponents[inSceneObjectComponent->GetClass().GetName()].insert(inSceneObjectComponent);
}

void Scene::RemoveSceneObjectComponent(SceneObjectComponentPtr inSceneObjectComponent)
{
	sceneObjectComponents[inSceneObjectComponent->GetClass().GetName()].erase(inSceneObjectComponent);
}

void Scene::PerFrameUpdate()
{
	float deltaTime = TimeManager::GetInstance()->GetDeltaTime();
	for (SceneObjectBasePtr sceneObject : sceneObjectsSet)
	{
		if (sceneObject->isTickEnabled)
		{
			sceneObject->Tick(deltaTime);
			sceneObject->TickComponents(deltaTime);
		}
	}

	// DIRTY TESTING SCENE
	MeshComponentPtr meshComp = GetSceneComponent<MeshComponent>();
	meshComp->GetParent()->transform.AddRotation({ 0.0f, deltaTime * 45.0f, deltaTime * 15.0f });
}
