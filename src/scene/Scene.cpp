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
#include "utils/Math3D.h"

namespace CGE
{

	namespace
	{
		constexpr const uint32_t OCTREE_NODE_POOL_SIZE = 250'000;
	}

	//-----------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------

	uint8_t CalculateTransformCellIndex(SceneObjectBasePtr object, OctreeNode<SceneObjectBasePtr>* node)
	{
		return node->CalculatePointSubnodeIndex(object->transform.GetLocation());
	}

	//-----------------------------------------------------------------------------------------------------------------

	bool IsNodeInFrustum(const Frustum& object, OctreeNode<SceneObjectBasePtr>* node)
	{
		AABB aabb;
		aabb.min = node->position;
		aabb.max = aabb.min + node->size;
		return FrustumIntersectSlow(object, aabb);
	}

	//-----------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------

	template<>
	struct OctreeNodePayload<SceneObjectBasePtr>
	{
		SceneObjectsPack objectsPack;

		void Add(SceneObjectBasePtr object)
		{
			objectsPack.Add(object);
		}
		void Remove(SceneObjectBasePtr object)
		{
			objectsPack.Remove(object);
		}
		void WriteOutput(SceneObjectsPack& outPack)
		{
			for (auto it = objectsPack.objectsList.begin(); it != objectsPack.objectsList.end(); it++)
			{
				outPack.Add(*it);
			}
		}
		bool IsEmpty()
		{ 
			return objectsPack.objectsList.empty(); 
		}
	};

	//-----------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------

	void SceneObjectsPack::Add(SceneObjectBasePtr object)
	{
		objectsList.insert(object);
		for (SceneObjectComponentPtr comp : object->GetComponents())
		{
			componentsMap[comp->GetClass().GetName()].insert(comp);
		}
		objectsMap[object->GetClass().GetName()].insert(object);
	}

	//-----------------------------------------------------------------------------------------------------------------

	void SceneObjectsPack::Remove(SceneObjectBasePtr object)
	{
		objectsList.erase(object);
		objectsMap[object->GetClass().GetName()].erase(object);
		for (SceneObjectComponentPtr comp : object->GetComponents())
		{
			componentsMap[comp->GetClass().GetName()].erase(comp);
		}
	}

	//-----------------------------------------------------------------------------------------------------------------

	void SceneObjectsPack::Clear()
	{
		objectsList.clear();
		objectsMap.clear();
		componentsMap.clear();
	}

	//-----------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------

	Scene::Scene()
	{
		m_sceneTree = new Octree<SceneObjectBasePtr>(OCTREE_NODE_POOL_SIZE, &CalculateTransformCellIndex);
	}

	//-----------------------------------------------------------------------------------------------------------------
	
	Scene::~Scene()
	{
	}

	//-----------------------------------------------------------------------------------------------------------------
	
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
		m_modelMatrices.resize(g_GlobalTransformDataSize);
		m_previousModelMatrices.resize(g_GlobalTransformDataSize);
	
		TransferList* tl = TransferList::GetInstance();
	
		//Texture2DPtr albedo = DataManager::RequestResourceType<Texture2D>("content/meshes/gun/Textures/Cerberus_A.tga", false, true, false);
		//Texture2DPtr normal = DataManager::RequestResourceType<Texture2D>("content/meshes/gun/Textures/Cerberus_N.tga", false, true, true);
		//Texture2DPtr albedo = DataManager::RequestResourceType<Texture2D>("content/meshes/uv_base.png", false, true, false);
		Texture2DPtr white = DataManager::RequestResourceType<Texture2D>("content/textures/white.png", false, true, false);
		Texture2DPtr red = DataManager::RequestResourceType<Texture2D>("content/textures/red.png", false, true, false);
		Texture2DPtr green = DataManager::RequestResourceType<Texture2D>("content/textures/green.png", false, true, false);
		//Texture2DPtr albedo = DataManager::RequestResourceType<Texture2D>("content/meshes/root/Aset_wood_root_M_rkswd_4K_Albedo.jpg", false, true, false, true);
		//Texture2DPtr normal = DataManager::RequestResourceType<Texture2D>("content/meshes/root/Aset_wood_root_M_rkswd_4K_Normal_LOD0.jpg", false, true, true, true);
		Texture2DPtr normal = DataManager::RequestResourceType<Texture2D>("content/textures/normals_flat.png", false, true, true, true);
		tl->PushImage(white);
		tl->PushImage(red);
		tl->PushImage(green);
		tl->PushImage(normal);
	
		//------------------------------------------------------------------------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------------------------------------------------------------------------
		// materials

		MaterialPtr mat = DataManager::RequestResourceType<Material>(
			"default",
			"content/shaders/GBufferVert.spv",
			"content/shaders/GBufferFrag.spv"
			);
		mat->SetTexture("albedo", white);
		mat->SetTexture("normal", normal);
		mat->LoadResources();

		MaterialPtr mat_red = DataManager::RequestResourceType<Material>(
			"red",
			"content/shaders/GBufferVert.spv",
			"content/shaders/GBufferFrag.spv"
			);
		mat_red->SetTexture("albedo", red);
		mat_red->SetTexture("normal", normal);
		mat_red->LoadResources();

		MaterialPtr mat_green = DataManager::RequestResourceType<Material>(
			"green",
			"content/shaders/GBufferVert.spv",
			"content/shaders/GBufferFrag.spv"
			);
		mat_green->SetTexture("albedo", green);
		mat_green->SetTexture("normal", normal);
		mat_green->LoadResources();

		//------------------------------------------------------------------------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------------------------------------------------------------------------
	
		Renderer* renderer = Engine::GetRendererInstance();
		// hardcoding dirty sample scene 
		CameraObjectPtr cameraObj = ObjectBase::NewObject<CameraObject>();
		cameraObj->transform.SetLocation({ 0.0f, -25.0f, 55.0f });
		//cameraObj->transform.SetRotation({ -10.0f, -180.0f, 0.0f });
		//cameraObj->transform.SetLocation({ 0.0f, 0.0f, -45.0f });
		cameraObj->transform.SetRotation({ -15.0f, -180.0f, 0.0f });
		cameraObj->GetCameraComponent()->SetFov(60.0f);
		cameraObj->GetCameraComponent()->SetNearPlane(0.1f);
		cameraObj->GetCameraComponent()->SetFarPlane(5000.0f);
		cameraObj->GetCameraComponent()->SetAspectRatio(float(renderer->GetWidth()) / float(renderer->GetHeight()));
	
		LightObjectPtr lightObj = ObjectBase::NewObject<LightObject>();
		lightObj->transform.SetLocation({ 0.0f, 0.0f, 0.0f });
		lightObj->transform.SetRotation({ -20.0f, -115.0f, 0.0f });
		lightObj->GetLightComponent()->type = LT_Directional;
		lightObj->GetLightComponent()->intensity = 1.6f;
		lightObj->GetLightComponent()->color = { 1.0f, 1.0f, 0.6f };
	

		//LightObjectPtr lightObj01 = ObjectBase::NewObject<LightObject>();
		//lightObj01->transform.SetLocation({ -15.0f, -1.0f, 5.0f });
		//lightObj01->transform.SetRotation({ 0.0f, 90.0f, 0.0f });
		//lightObj01->GetLightComponent()->type = LT_Spot;
		//lightObj01->GetLightComponent()->radius = 115.0f;
		//lightObj01->GetLightComponent()->spotHalfAngle = 30.0f;
		//lightObj01->GetLightComponent()->intensity = 2.0f;
		//lightObj01->GetLightComponent()->color = { 0.2f, 1.0f, 0.2f };
	
		float width = 500.0f;
		float depth = 500.0f;
		uint32_t counter = 0;
		uint32_t countX = 15;
		uint32_t countY = 15;
		for (uint32_t indexX = 0; indexX < countX; indexX++)
		{
			for (uint32_t indexY = 0; indexY < countY; indexY++)
			{
				glm::vec3 color = counter % 2 == 0 ? glm::vec3{1.0f, 0.0f, 0.0f} : (counter % 3 == 2) ? glm::vec3{0.0f, 1.0f, 0.0f} : glm::vec3{0.0f, 0.0f, 1.0f};
				bool isSpot = false;// counter % 2;
	
				//LightObjectPtr lightObj02 = ObjectBase::NewObject<LightObject>();
				//lightObj02->transform.SetLocation({ -width * 0.5f + ((indexX + 0.5f) * width / float(countX - 1)), -10.0f, -1.0 * (indexY) * depth / float(countY - 1) });
				//lightObj02->transform.SetRotation({ 90.0f, 0.0f, 0.0f });
				//lightObj02->GetLightComponent()->type = isSpot ? LT_Spot : LT_Point;
				//lightObj02->GetLightComponent()->radius = isSpot ? 60.0f : 35.0f;
				//lightObj02->GetLightComponent()->spotHalfAngle = 20.0f;
				//lightObj02->GetLightComponent()->intensity = isSpot ? 35.0f : 1.0f;
				//lightObj02->GetLightComponent()->color = color;
	
				++counter;
			}
		}
	
		{
			RtMaterialPtr rtMat1 = DataManager::RequestResourceType<RtMaterial>("rt_mat1");
			rtMat1->SetShader(ERtShaderType::RST_CLOSEST_HIT, "content/shaders/RayClosestHitDefault1.spv", "main");
			rtMat1->LoadResources();

			RtMaterialPtr rtMat2 = DataManager::RequestResourceType<RtMaterial>("rt_mat2");
			rtMat2->SetShader(ERtShaderType::RST_CLOSEST_HIT, "content/shaders/RayClosestHitDefault2.spv", "main");
			rtMat2->LoadResources();

			MeshImporter importer;
			//importer.Import("./content/meshes/gun/Cerberus_LP.FBX");
			//importer.Import("./content/meshes/root/Aset_wood_root_M_rkswd_LOD0.FBX");
			//importer.Import("./content/meshes/cube/cube.fbx");
			importer.Import("./content/meshes/rooms/room_01.fbx");
			for (unsigned int MeshIndex = 0; MeshIndex < importer.GetMeshes().size(); MeshIndex++)
			{
				MeshDataPtr meshData = importer.GetMeshes()[MeshIndex];
				meshData->CreateBuffer();
	
				float width = 500.0f;
				float depth = 500.0f;
				uint32_t countX = 1;
				uint32_t countY = 1;
				for (uint32_t indexX = 0; indexX < countX; indexX++)
				{
					for (uint32_t indexY = 0; indexY < countY; indexY++)
					{
						float randomY = std::rand() / float(RAND_MAX);
						float randomZ = std::rand() / float(RAND_MAX);
	
						MeshObjectPtr mo3 = ObjectBase::NewObject<MeshObject>();
						mo3->GetMeshComponent()->meshData = meshData;
						//mo3->transform.SetLocation({ -width * 0.5f + (indexX * width / float(countX - 1)), 0.0f, -1.0 * indexY * depth / float(countY - 1) });
						mo3->transform.SetLocation({ 0.0f, 0.0f, 0.0f });
						//mo3->transform.SetRotation({ randomZ * 180.0f, 0.0f, 90.0 });
						mo3->transform.SetRotation({ 0.0f, 0.0f, 0.0 });
						mo3->transform.SetScale({ 1.0f, 1.0f, 1.0f });
						mo3->GetMeshComponent()->SetMaterial(mat);
						mo3->GetMeshComponent()->SetRtMaterial(indexY % 2 ? rtMat1 : rtMat2);
					}
				}
			}

			MeshImporter cube;
			//importer.Import("./content/meshes/gun/Cerberus_LP.FBX");
			//importer.Import("./content/meshes/root/Aset_wood_root_M_rkswd_LOD0.FBX");
			//importer.Import("./content/meshes/cube/cube.fbx");
			cube.Import("./content/meshes/cube/cube.fbx");
			MeshDataPtr meshData = cube.GetMeshes()[0];
			meshData->CreateBuffer();


			float width = 15.0f;
			float depth = 15.0f;
			uint32_t countX = 2;
			uint32_t countY = 2;
			for (uint32_t indexX = 0; indexX < countX; indexX++)
			{
				for (uint32_t indexY = 0; indexY < countY; indexY++)
				{
					float randomY = std::rand() / float(RAND_MAX);
					float randomZ = std::rand() / float(RAND_MAX);

					MeshObjectPtr mo3 = ObjectBase::NewObject<MeshObject>();
					mo3->GetMeshComponent()->meshData = meshData;
					mo3->transform.SetLocation({ -width * 0.5f + (indexX * width / float(countX - 1)), -10.0f, -1.0 * indexY * depth / float(countY - 1) });
					//mo3->transform.SetLocation({ 0.0f, 0.0f, 0.0f });
					//mo3->transform.SetRotation({ randomZ * 180.0f, 0.0f, 90.0 });
					mo3->transform.SetRotation({ 0.0f, 0.0f, 0.0 });
					mo3->transform.SetScale({ 2.0f, 5.0f, 2.0f });
					mo3->GetMeshComponent()->SetMaterial(mat);//indexX % 2 ? mat_red : mat_green);
					mo3->GetMeshComponent()->SetRtMaterial(indexY % 2 ? rtMat1 : rtMat2);
				}
			}
		}
	
		MeshData::FullscreenQuad()->CreateBuffer();

		for (SceneObjectBasePtr objPtr : m_primaryPack.objectsList)
		{
			m_sceneTree->AddObject(objPtr);
		}
		m_sceneTree->Update();
	}

	//-----------------------------------------------------------------------------------------------------------------
	
	void Scene::RegisterSceneObject(SceneObjectBasePtr inSceneObject)
	{
		m_primaryPack.objectsList.insert(inSceneObject);
		m_primaryPack.objectsMap[inSceneObject->GetClass().GetName()].insert(inSceneObject);
	}

	//-----------------------------------------------------------------------------------------------------------------
	
	void Scene::RemoveSceneObject(SceneObjectBasePtr inSceneObject)
	{
		m_primaryPack.objectsList.erase(inSceneObject);
		m_primaryPack.objectsMap[inSceneObject->GetClass().GetName()].erase(inSceneObject);
	}

	//-----------------------------------------------------------------------------------------------------------------
	
	void Scene::RegisterSceneObjectComponent(SceneObjectComponentPtr inSceneObjectComponent)
	{
		m_primaryPack.componentsMap[inSceneObjectComponent->GetClass().GetName()].insert(inSceneObjectComponent);
	}

	//-----------------------------------------------------------------------------------------------------------------
	
	void Scene::RemoveSceneObjectComponent(SceneObjectComponentPtr inSceneObjectComponent)
	{
		m_primaryPack.componentsMap[inSceneObjectComponent->GetClass().GetName()].erase(inSceneObjectComponent);
	}

	//-----------------------------------------------------------------------------------------------------------------
	
	void Scene::PrepareObjectsLists()
	{
		// testing scene tree agains camera frustum
		GatherObjectsInFrustum();

		/*
		single threaded simple scene data processing for batching and instancing. later it'll become multi threaded procedure
		*/
		m_shadersList.clear();
		m_shaderToMaterial.clear();
		m_materialToMeshData.clear();
		m_matToMeshToTransform.clear();
		m_materialToMeshDataToIndex.clear();
	
		const Class& meshDataClass = Class::Get<MeshComponent>();
		std::vector<MeshComponentPtr> meshes = GetSceneComponentsCast<MeshComponent>(m_frustumPack);
		for (MeshComponentPtr meshComponent : meshes)
		{
			MaterialPtr material = meshComponent->material;
			MeshDataPtr meshData = meshComponent->meshData;

			HashString shaderHash = material->GetShaderHash();
			HashString materialId = material->GetResourceId();
			HashString meshDataId = meshData->GetResourceId();

			if (m_shaderToMaterial.find(shaderHash) == m_shaderToMaterial.end())
			{
				m_shadersList.push_back(shaderHash);
			}
			if (m_materialToMeshData.find(materialId) == m_materialToMeshData.end())
			{
				m_shaderToMaterial[shaderHash].push_back(material);
			}
			if (m_matToMeshToTransform[materialId].find(meshDataId) == m_matToMeshToTransform[materialId].end())
			{
				m_materialToMeshData[materialId].push_back(meshData);
			}

			auto& transform = meshComponent->GetParent()->transform;
			MatrixPair pair;
			pair.previousMatrix = transform.GetMemorizedTransformMatrix();
			pair.matrix = transform.CalculateMatrix();

			m_matToMeshToTransform[materialId][meshDataId].emplace_back(pair);

			transform.MemorizeTransformMatrix();
		}

		uint32_t counter = 0;
		for (HashString& shaderHash : m_shadersList)
		{
			for (MaterialPtr material : m_shaderToMaterial[shaderHash])
			{
				for (MeshDataPtr meshData : m_materialToMeshData[material->GetResourceId()])
				{
					m_materialToMeshDataToIndex[material->GetResourceId()][meshData->GetResourceId()] = counter;
					for (auto& matrixPair : m_matToMeshToTransform[material->GetResourceId()][meshData->GetResourceId()])
					{
						m_modelMatrices[counter] = matrixPair.matrix;
						m_previousModelMatrices[counter] = matrixPair.previousMatrix;
						++counter;
					}
				}
			}
		}
		m_relevantMatricesCount = counter;
	}

	//-----------------------------------------------------------------------------------------------------------------
	
	void Scene::PerFrameUpdate()
	{
		float deltaTime = TimeManager::GetInstance()->GetDeltaTime();
		/*for (SceneObjectBasePtr sceneObject : sceneObjectsSet)
		{
			if (sceneObject->isTickEnabled)
			{
				sceneObject->Tick(deltaTime);
				sceneObject->TickComponents(deltaTime);
			}
		}*/
	
		// DIRTY TESTING SCENE
		std::vector<MeshComponentPtr> meshComps = GetSceneComponentsCast<MeshComponent>();
		for (MeshComponentPtr meshComp : meshComps)
		{
			float multiplierY = deltaTime * 10.0f;
			float multiplierZ = deltaTime * 10.0f;
			//meshComp->GetParent()->transform.AddRotation({ multiplierY, 0.0f, 0.0f });
		}

		std::vector<LightComponentPtr> lights = GetSceneComponentsCast<LightComponent>();
		for (LightComponentPtr light : lights)
		{

			light->GetParent()->transform.AddRotation({ 0.0f, 0.05f, 0.0f });
		}

		double time = TimeManager::GetInstance()->GetTime();
		CameraComponentPtr cam = GetSceneComponent<CameraComponent>(m_primaryPack);
		if (cam)
		{

//			cam->GetParent()->transform.SetRotation({-15.0f, 180.0f + 25.0f * glm::sin(time / 10.0f), 0.0f});
		}
	
		PrepareObjectsLists();
	}

	//-----------------------------------------------------------------------------------------------------------------

	SceneObjectsPack& Scene::GetObjectsPack(bool isFrustumCulled)
	{
		return isFrustumCulled ? m_frustumPack : m_primaryPack;
	}

	//-----------------------------------------------------------------------------------------------------------------

	void Scene::GatherObjectsInFrustum()
	{
		auto startTime = std::chrono::high_resolution_clock::now();

		m_frustumPack.Clear();
		//Frustum frustum = CreateFrustum(GetSceneComponent<CameraComponent>(primaryPack));
		CameraComponentPtr cam = GetSceneComponent<CameraComponent>(m_primaryPack);
		Transform& tr = cam->GetParent()->transform;
		float extendedFrustumOffset = 50.0f;
		Frustum frustum = CreateFrustum(
			tr.GetLocation() - tr.GetForwardVector() * extendedFrustumOffset,
			tr.GetForwardVector(),
			tr.GetUpVector(),
			tr.GetLeftVector(),
			cam->GetNearPlane(),
			cam->GetFarPlane() + extendedFrustumOffset,
			cam->GetFov(),
			cam->GetAspectRatio()
		);
		m_sceneTree->Query<Frustum, SceneObjectsPack>(frustum, IsNodeInFrustum, m_frustumPack);

		auto currentTime = std::chrono::high_resolution_clock::now();
		double deltaTime = std::chrono::duration<double, std::chrono::microseconds::period>(currentTime - startTime).count();
		//std::printf("scene frustum query time is %f microseconds\n", deltaTime);
	}

}


