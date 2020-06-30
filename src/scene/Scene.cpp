#include "scene/Scene.h"
#include "scene/SceneObjectBase.h"
#include "scene/SceneObjectComponent.h"

#include "core/TimeManager.h"
#include "camera/CameraObject.h"
#include "mesh/MeshObject.h"
#include "import/MeshImporter.h"
#include "render/TransferList.h"
#include "data/DataManager.h"
#include "render/DataStructures.h"
#include "light/LightObject.h"

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
	TransferList* tl = TransferList::GetInstance();

	Texture2DPtr albedo = DataManager::RequestResourceType<Texture2D, bool, bool, bool>("content/meshes/gun/Textures/Cerberus_A.tga", false, true, false);
	Texture2DPtr normal = DataManager::RequestResourceType<Texture2D, bool, bool, bool>("content/meshes/gun/Textures/Cerberus_N.tga", false, true, false);
	tl->PushImage(&albedo->GetImage());
	tl->PushImage(&normal->GetImage());

	MaterialPtr mat = DataManager::RequestResourceType<Material>(
		"default",
		"content/shaders/GBufferVert.spv",
		"content/shaders/GBufferFrag.spv"
		);
	mat->SetTexture("albedo", albedo);
	mat->SetTexture("normal", normal);
	ObjectMVPData objData;
	mat->SetUniformBuffer<ObjectMVPData>("mvpBuffer", objData);
	mat->LoadResources();

	MaterialPtr mat2 = DataManager::RequestResourceType<Material>(
		"default2",
		"content/shaders/GBufferVert.spv",
		"content/shaders/GBufferFrag.spv"
		);
	mat2->SetTexture("normal", albedo);
	mat2->SetTexture("albedo", normal);
	mat2->SetUniformBuffer<ObjectMVPData>("mvpBuffer", objData);
	mat2->LoadResources();

	MaterialPtr mat3 = DataManager::RequestResourceType<Material>(
		"default3",
		"content/shaders/GBufferVert.spv",
		"content/shaders/GBufferFrag.spv"
		);
	mat3->SetTexture("albedo", albedo);
	mat3->SetTexture("normal", normal);
	mat3->SetUniformBuffer<ObjectMVPData>("mvpBuffer", objData);
	mat3->LoadResources();

	// hardcoding dirty sample scene 
	CameraObjectPtr cameraObj = ObjectBase::NewObject<CameraObject>();
	cameraObj->transform.SetLocation({ 0.0f, 0.0f, 35.0f });
	cameraObj->transform.SetRotation({ 0.0f, 180.0f, 0.0f });
	cameraObj->GetCameraComponent()->SetFov(110.0f);
	cameraObj->GetCameraComponent()->SetNearPlane(0.1f);
	cameraObj->GetCameraComponent()->SetFarPlane(100.0f);

	LightObjectPtr lightObj = ObjectBase::NewObject<LightObject>();
	lightObj->transform.SetLocation({ -10.0f, 0.0f, 0.0f });
	lightObj->transform.SetRotation({ 0.0f, 0.0f, 0.0f });
	lightObj->GetLightComponent()->type = LT_Point;
	lightObj->GetLightComponent()->radius = 20.0f;
	lightObj->GetLightComponent()->spotHalfAngle = 15.0f;
	lightObj->GetLightComponent()->intensity = 1.0f;
	lightObj->GetLightComponent()->color = {1.0f, 1.0f, 1.0f};

	LightObjectPtr lightObj01 = ObjectBase::NewObject<LightObject>();
	lightObj01->transform.SetLocation({ 20.0f, 10.0f, 0.0f });
	lightObj01->transform.SetRotation({ 85.0f, -90.0f, 0.0f });
	lightObj01->GetLightComponent()->type = LT_Spot;
	lightObj01->GetLightComponent()->radius = 25.0f;
	lightObj01->GetLightComponent()->spotHalfAngle = 25.0f;
	lightObj01->GetLightComponent()->intensity = 1.0f;
	lightObj01->GetLightComponent()->color = { 1.0f, 1.0f, 1.0f };

	{
		MeshImporter importer;
		//importer.Import("./content/root/Aset_wood_root_M_rkswd_LOD0.FBX");
		importer.Import("./content/meshes/gun/Cerberus_LP.FBX");
		for (unsigned int MeshIndex = 0; MeshIndex < importer.GetMeshes().size(); MeshIndex++)
		{
			MeshDataPtr meshData = importer.GetMeshes()[MeshIndex];
			meshData->CreateBuffer();
			tl->PushBuffers(meshData);

			MeshObjectPtr mo = ObjectBase::NewObject<MeshObject>();
			mo->GetMeshComponent()->meshData = meshData;
			mo->transform.SetLocation({ 25.0f, -5.0f, 0.0f });
			mo->transform.SetScale({ 0.4f, 0.4f, 0.4f });
			mo->GetMeshComponent()->SetMaterial(mat);

			MeshObjectPtr mo2 = ObjectBase::NewObject<MeshObject>();
			mo2->GetMeshComponent()->meshData = meshData;
			mo2->transform.SetLocation({ -15.0f, -5.0f, 0.0f });
			mo2->transform.SetScale({ 0.4f, 0.4f, 0.4f });
			mo2->GetMeshComponent()->SetMaterial(mat2);

			MeshObjectPtr mo3 = ObjectBase::NewObject<MeshObject>();
			mo3->GetMeshComponent()->meshData = meshData;
			mo3->transform.SetLocation({ 0.0f, 0.0f, -65.0f });
			mo3->transform.SetScale({ 0.4f, 0.4f, 0.4f });
			mo3->GetMeshComponent()->SetMaterial(mat3);
		}

	}

	MeshData::FullscreenQuad()->CreateBuffer();
	tl->PushBuffers(MeshData::FullscreenQuad());
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
	std::vector<MeshComponentPtr> meshComps = GetSceneComponentsCast<MeshComponent>();
	uint32_t index = 1;
	for (MeshComponentPtr meshComp : meshComps)
	{
		meshComp->GetParent()->transform.AddRotation({ 0.0f, deltaTime * 15.0f * index, deltaTime * 5.0f * index });
		ObjectMVPData ubo;
		ubo.model = meshComp->GetParent()->transform.GetMatrix();

		meshComp->material->UpdateUniformBuffer<ObjectMVPData>("mvpBuffer", ubo);

		index++;
	}
}
