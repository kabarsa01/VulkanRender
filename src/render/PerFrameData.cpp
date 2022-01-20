#include "PerFrameData.h"
#include "objects/VulkanDevice.h"
#include "GlobalSamplers.h"
#include "scene/Scene.h"
#include "core/Engine.h"
#include "scene/camera/CameraComponent.h"
#include "core/TimeManager.h"
#include "scene/SceneObjectComponent.h"
#include "scene/Transform.h"
#include "scene/SceneObjectBase.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "ClusteringManager.h"
#include "utils/Singleton.h"
#include "utils/ResourceUtils.h"
#include "data/DataManager.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::SharingMode;
	using VULKAN_HPP_NAMESPACE::BufferUsageFlagBits;
	using VULKAN_HPP_NAMESPACE::DescriptorType;
	using VULKAN_HPP_NAMESPACE::ShaderStageFlagBits;

	PerFrameData::PerFrameData()
	{
	}
	
	PerFrameData::~PerFrameData()
	{
	}
	
	void PerFrameData::Create(VulkanDevice* inDevice)
	{
		device = inDevice;
	
		GlobalSamplers::GetInstance()->SetMipLodBias(-0.75f);
		GlobalSamplers::GetInstance()->Create(device);
	
		m_globalShaderData = new GlobalShaderData();
		m_globalTransformData = new GlobalTransformData();
		m_globalPreviousTransformData = new GlobalTransformData();

		m_transformDataBuffer = ResourceUtils::CreateBufferData("global_transform_data_", sizeof(GlobalTransformData), vk::BufferUsageFlagBits::eStorageBuffer, true);
		m_transformPreviousDataBuffer = ResourceUtils::CreateBufferData("global_previous_transform_data_", sizeof(GlobalTransformData), vk::BufferUsageFlagBits::eStorageBuffer, true);

		m_globalDataBuffer = ResourceUtils::CreateBufferData("PerFrameShaderData_", sizeof(GlobalShaderData), BufferUsageFlagBits::eUniformBuffer, true);
		m_globalPreviousDataBuffer = ResourceUtils::CreateBufferData("PerFramePreviousShaderData_", sizeof(GlobalShaderData), BufferUsageFlagBits::eUniformBuffer, true);

		m_data.resize(2);
		
		uint32_t counter = 0;
		for (auto& frameData : m_data)
		{
			ShaderPtr shader = DataManager::GetInstance()->RequestResourceByType<Shader>("content/shaders/GBufferVert.spv");

			auto& resMapper = frameData.resourceMapper;
			resMapper.AddUniformBuffer("globalData", m_globalDataBuffer);
			resMapper.AddUniformBuffer("globalPreviousData", m_globalPreviousDataBuffer);
			resMapper.AddUniformBuffer("globalTransformData", m_transformDataBuffer);
			resMapper.AddUniformBuffer("globalPreviousTransformData", m_transformPreviousDataBuffer);
			resMapper.AddShader(shader);
			resMapper.Update();

			frameData.m_set = resMapper.GetDescriptorSets()[0];

			++counter;
		}
	}
	
	void PerFrameData::Destroy()
	{
		delete m_globalShaderData;
		delete m_globalTransformData;
		delete m_globalPreviousTransformData;
		
		for (auto& data : m_data)
		{
			data.resourceMapper.Destroy();
		}
		
		GlobalSamplers::GetInstance()->Destroy();
	}
	
	void PerFrameData::UpdateBufferData()
	{
		m_globalPreviousDataBuffer->CopyTo(sizeof(GlobalShaderData), reinterpret_cast<const char*>( m_globalShaderData ));

		GatherData();

		m_globalDataBuffer->CopyTo(sizeof(GlobalShaderData), reinterpret_cast<const char*>( m_globalShaderData ));
		m_transformDataBuffer->CopyTo(m_relevantTransformsSize, reinterpret_cast<const char*>( m_globalTransformData ));
		m_transformPreviousDataBuffer->CopyTo(m_relevantTransformsSize, reinterpret_cast<const char*>( m_globalPreviousTransformData ));
	}

	void PerFrameData::GatherData()
	{
		m_globalShaderData->time = TimeManager::GetInstance()->GetTime();
		m_globalShaderData->deltaTime = TimeManager::GetInstance()->GetDeltaTime();
	
		Scene* scene = Engine::GetSceneInstance();
		CameraComponentPtr camComp = scene->GetSceneComponent<CameraComponent>();
	
		m_globalShaderData->worldToView = camComp->CalculateViewMatrix();
		m_globalShaderData->viewToProj = camComp->CalculateProjectionMatrix();
		m_globalShaderData->cameraPos = camComp->GetParent()->transform.GetLocation();
		m_globalShaderData->viewVector = camComp->GetParent()->transform.GetForwardVector();

		m_globalShaderData->numClusters = Singleton<ClusteringManager>::GetInstance()->GetNumClusters();
		m_globalShaderData->clusterSize = Singleton<ClusteringManager>::GetInstance()->GetClusterSize();
		m_globalShaderData->halfScreenOffset = Singleton<ClusteringManager>::GetInstance()->GetHalfScreenOffset();
		m_globalShaderData->clusterScreenOverflow = Singleton<ClusteringManager>::GetInstance()->GetClusterScreenOverflow();

		m_globalShaderData->cameraNear = camComp->GetNearPlane();
		m_globalShaderData->cameraFar = camComp->GetFarPlane();
		m_globalShaderData->cameraFov = camComp->GetFov();
		m_globalShaderData->cameraAspect = camComp->GetAspectRatio();
	
		m_relevantTransformsSize = scene->GetRelevantMatricesCount() * sizeof(glm::mat4x4);
		std::memcpy(m_globalTransformData->modelToWorld, scene->GetModelMatrices().data(), m_relevantTransformsSize);
		std::memcpy(m_globalPreviousTransformData->modelToWorld, scene->GetPreviousModelMatrices().data(), m_relevantTransformsSize);
	}
}

