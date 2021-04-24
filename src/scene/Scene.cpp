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
#include "async/ThreadPool.h"
#include "async/Job.h"
#include <iostream>
#include "messages/MessageHandler.h"
#include "messages/MessageBus.h"
#include "messages/MessageSubscriber.h"
#include "core/ObjectPool.h"
#include "scene/Octree.h"

namespace CGE
{

	namespace
	{
		constexpr const uint32_t OCTREE_NODE_POOL_SIZE = 250'000;
	}

	uint8_t CalculateTransformCellIndex(SceneObjectBasePtr object, OctreeNode<SceneObjectBasePtr>* node)
	{
		return node->CalculatePointSubnodeIndex(object->transform.GetLocation());
	}

	Scene::Scene()
		: ObjectBase()
	{
		sceneTree = new Octree<SceneObjectBasePtr>(OCTREE_NODE_POOL_SIZE, &CalculateTransformCellIndex);
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
		//for (int i = 0; i < 100; i++)
		//{
		//	ThreadPool::GetInstance()->AddJob(std::make_shared<Job<void()>>([]() { for (int idx = 0; idx < 10; idx++)
		//	{
		//		std::this_thread::sleep_for(std::chrono::seconds(1));
		//		std::cout << "thread " << std::this_thread::get_id() << " :: index " << idx << std::endl;
		//	}
		//	 }));
		//}

		{
			struct Doop
			{
				void haldn(std::shared_ptr<GlobalUpdateMessage> msg)
				{
					std::cout << "" << msg->GetId() << " :: delta time :: " << msg->deltaTime << std::endl;
				}

				void haldn(std::shared_ptr<SceneProcessingFinishedMessage> msg)
				{
					std::cout << "" << msg->GetId() << std::endl;
				}
			};

			Doop doop;

			MessageSubscriber subscriber;
			subscriber.AddHandler<GlobalUpdateMessage>(&doop, &Doop::haldn);
			subscriber.AddHandler<SceneProcessingFinishedMessage>(&doop, &Doop::haldn);

			std::shared_ptr<GlobalUpdateMessage> msg1 = std::make_shared<GlobalUpdateMessage>();
			msg1->deltaTime = 16.6f;
			std::shared_ptr<GlobalUpdateMessage> msg2 = std::make_shared<GlobalUpdateMessage>();
			msg2->deltaTime = 33.2f;
			std::shared_ptr<SceneProcessingFinishedMessage> msg3 = std::make_shared<SceneProcessingFinishedMessage>();

			for (uint32_t i = 0; i < 5; i++)
			{
				MessageBus::GetInstance()->PublishSync(msg1, msg2, msg3);
			}
		}

		modelMatrices.resize(g_GlobalTransformDataSize);
	
		TransferList* tl = TransferList::GetInstance();
	
		//Texture2DPtr albedo = DataManager::RequestResourceType<Texture2D>("content/meshes/gun/Textures/Cerberus_A.tga", false, true, false);
		//Texture2DPtr normal = DataManager::RequestResourceType<Texture2D>("content/meshes/gun/Textures/Cerberus_N.tga", false, true, true);
		//Texture2DPtr albedo = DataManager::RequestResourceType<Texture2D>("content/meshes/uv_base.png", false, true, false);
		Texture2DPtr albedo = DataManager::RequestResourceType<Texture2D>("content/meshes/root/Aset_wood_root_M_rkswd_4K_Albedo.jpg", false, true, false, true);
		Texture2DPtr normal = DataManager::RequestResourceType<Texture2D>("content/meshes/root/Aset_wood_root_M_rkswd_4K_Normal_LOD0.jpg", false, true, true, true);
		tl->PushImage(&albedo->GetImage());
		tl->PushImage(&normal->GetImage());
	
		MaterialPtr mat = DataManager::RequestResourceType<Material>(
			"default",
			"content/shaders/GBufferVert.spv",
			"content/shaders/GBufferFrag.spv"
			);
		mat->SetTexture("albedo", albedo);
		mat->SetTexture("normal", normal);
		mat->LoadResources();
	
		Renderer* renderer = Engine::GetRendererInstance();
		// hardcoding dirty sample scene 
		CameraObjectPtr cameraObj = ObjectBase::NewObject<CameraObject>();
		cameraObj->transform.SetLocation({ 0.0f, -25.0f, 25.0f });
		cameraObj->transform.SetRotation({ -30.0f, 180.0f, 0.0f });
		cameraObj->GetCameraComponent()->SetFov(90.0f);
		cameraObj->GetCameraComponent()->SetNearPlane(0.1f);
		cameraObj->GetCameraComponent()->SetFarPlane(4000.0f);
		cameraObj->GetCameraComponent()->SetAspectRatio(float(renderer->GetWidth()) / float(renderer->GetHeight()));
	
		LightObjectPtr lightObj = ObjectBase::NewObject<LightObject>();
		lightObj->transform.SetLocation({ 0.0f, 0.0f, 0.0f });
		lightObj->transform.SetRotation({ -40.0f, -90.0f, 0.0f });
		lightObj->GetLightComponent()->type = LT_Directional;
		lightObj->GetLightComponent()->intensity = 1.6f;
		lightObj->GetLightComponent()->color = { 1.0f, 1.0f, 0.6f };
	
		//LightObjectPtr lightObj01 = ObjectBase::NewObject<LightObject>();
		//lightObj01->transform.SetLocation({ -125.0f, 0.0f, 0.0f });
		//lightObj01->transform.SetRotation({ 0.0f, 90.0f, 0.0f });
		//lightObj01->GetLightComponent()->type = LT_Spot;
		//lightObj01->GetLightComponent()->radius = 245.0f;
		//lightObj01->GetLightComponent()->spotHalfAngle = 30.0f;
		//lightObj01->GetLightComponent()->intensity = 5.0f;
		//lightObj01->GetLightComponent()->color = { 0.2f, 0.6f, 1.0f };
	
		float width = 100.0f;
		float depth = 65.0f;
		uint32_t counter = 0;
		for (uint32_t indexX = 0; indexX < 5; indexX++)
		{
			for (uint32_t indexY = 0; indexY < 5; indexY++)
			{
				glm::vec3 color = counter % 3 == 0 ? glm::vec3{1.0f, 0.0f, 0.0f} : (counter % 3 == 1) ? glm::vec3{0.0f, 1.0f, 0.0f} : glm::vec3{0.0f, 0.0f, 1.0f};
				bool isSpot = counter % 2;
	
				LightObjectPtr lightObj02 = ObjectBase::NewObject<LightObject>();
				lightObj02->transform.SetLocation({ -width * 0.5f + indexX * width / 4.0, isSpot ? 20.0f : -10.0f, -1.0 * indexY * depth / 4.0 });
				lightObj02->transform.SetRotation({ 90.0f, 0.0f, 0.0f });
				lightObj02->GetLightComponent()->type = isSpot ? LT_Spot : LT_Point;
				lightObj02->GetLightComponent()->radius = isSpot ? 60.0f : 5.0f;
				lightObj02->GetLightComponent()->spotHalfAngle = 20.0f;
				lightObj02->GetLightComponent()->intensity = isSpot ? 35.0f : 15.0f;
				lightObj02->GetLightComponent()->color = color;
	
				++counter;
			}
		}
	
		{
			MeshImporter importer;
			//importer.Import("./content/meshes/gun/Cerberus_LP.FBX");
			importer.Import("./content/meshes/root/Aset_wood_root_M_rkswd_LOD0.FBX");
			//importer.Import("./content/meshes/cube/cube.fbx");
			for (unsigned int MeshIndex = 0; MeshIndex < importer.GetMeshes().size(); MeshIndex++)
			{
				MeshDataPtr meshData = importer.GetMeshes()[MeshIndex];
				meshData->CreateBuffer();
				tl->PushBuffers(meshData);
	
				float width = 100;
				float depth = 65.0f;
				for (uint32_t indexX = 0; indexX < 5; indexX++)
				{
					for (uint32_t indexY = 0; indexY < 5; indexY++)
					{
						float randomY = std::rand() / float(RAND_MAX);
						float randomZ = std::rand() / float(RAND_MAX);
	
						MeshObjectPtr mo3 = ObjectBase::NewObject<MeshObject>();
						mo3->GetMeshComponent()->meshData = meshData;
						mo3->transform.SetLocation({ -width * 0.5f + indexX * width / 4.0, 0.0f, -1.0 * indexY * depth / 4.0 });
						//mo3->transform.SetLocation({ 0.0f, 0.0f, 0.0f });
						mo3->transform.SetRotation({ randomZ * 180.0f, 0.0f, 90.0 });
						mo3->transform.SetScale({ 1.0f, 1.0f, 1.0f });
						mo3->GetMeshComponent()->SetMaterial(mat);
					}
				}
			}
	
		}
	
		MeshData::FullscreenQuad()->CreateBuffer();
		tl->PushBuffers(MeshData::FullscreenQuad());

		for (SceneObjectBasePtr objPtr : sceneObjectsSet)
		{
			sceneTree->AddObject(objPtr);
		}
		sceneTree->Update();
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
	
	void Scene::PrepareObjectsLists()
	{
		/*
		single threaded simple scene data processing for batching and instancing. later it'll become multi threaded procedure
		with scene data stored in tree as it should
		*/
		shadersList.clear();
		shaderToMaterial.clear();
		materialToMeshData.clear();
		matToMeshToTransform.clear();
		materialToMeshDataToIndex.clear();
	
		const Class& meshDataClass = Class::Get<MeshComponent>();
		std::vector<MeshComponentPtr> meshes = GetSceneComponentsCast<MeshComponent>();
		for (MeshComponentPtr meshComponent : meshes)
		{
			MaterialPtr material = meshComponent->material;
			MeshDataPtr meshData = meshComponent->meshData;
	
			HashString shaderHash = material->GetShaderHash();
			HashString materialId = material->GetResourceId();
			HashString meshDataId = meshData->GetResourceId();
	
			if (shaderToMaterial.find(shaderHash) == shaderToMaterial.end())
			{
				shadersList.push_back(shaderHash);
			}
			if (materialToMeshData.find(materialId) == materialToMeshData.end())
			{
				shaderToMaterial[shaderHash].push_back(material);
			}
			if (matToMeshToTransform[materialId].find(meshDataId) == matToMeshToTransform[materialId].end())
			{
				materialToMeshData[materialId].push_back(meshData);
			}
			matToMeshToTransform[materialId][meshDataId].push_back(meshComponent->GetParent()->transform.CalculateMatrix());
		}
	
		uint32_t counter = 0;
		for (HashString& shaderHash : shadersList)
		{
			for (MaterialPtr material : shaderToMaterial[shaderHash])
			{
				for (MeshDataPtr meshData : materialToMeshData[material->GetResourceId()])
				{
					materialToMeshDataToIndex[material->GetResourceId()][meshData->GetResourceId()] = counter;
					for (glm::mat4& modelMatrix : matToMeshToTransform[material->GetResourceId()][meshData->GetResourceId()])
					{
						modelMatrices[counter++] = modelMatrix;
					}
				}
			}
		}
		relevantMatricesCount = counter;
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
		for (MeshComponentPtr meshComp : meshComps)
		{
			float multiplierY = deltaTime * 10.0f;
			float multiplierZ = deltaTime * 10.0f;
			meshComp->GetParent()->transform.AddRotation({ multiplierY, 0.0f, 0.0f });
		}
	
		PrepareObjectsLists();
	}
}
