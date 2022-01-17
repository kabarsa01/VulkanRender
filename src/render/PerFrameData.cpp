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

		std::vector<BufferDataPtr> transformDataBuffer = ResourceUtils::CreateBufferDataArray(
			"global_transform_data_", 
			2, 
			sizeof(GlobalTransformData), 
			vk::BufferUsageFlagBits::eStorageBuffer, 
			true);

		BufferDataPtr shaderDataBuffer = ResourceUtils::CreateBufferData("PerFrameShaderData_", sizeof(GlobalShaderData), BufferUsageFlagBits::eUniformBuffer, true);

		m_data.resize(2);
		
		uint32_t counter = 0;
		for (auto& frameData : m_data)
		{
			frameData.shaderDataBuffer = shaderDataBuffer;

			frameData.transformDataBuffer = transformDataBuffer[0];
			frameData.previousTransformDataBuffer = transformDataBuffer[1];

			frameData.m_set.SetBindings(ProduceBindings(frameData));
			frameData.m_set.Create(device);

			frameData.descriptorWrites = ProduceWrites(frameData);
			device->GetDevice().updateDescriptorSets(static_cast<uint32_t>(frameData.descriptorWrites.size()), frameData.descriptorWrites.data(), 0, nullptr);

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
			data.shaderDataBuffer = nullptr;
			data.transformDataBuffer = nullptr;
			data.previousTransformDataBuffer = nullptr;
			data.m_set.Destroy();
		}
		
		GlobalSamplers::GetInstance()->Destroy();
	}
	
	void PerFrameData::UpdateBufferData()
	{
		GatherData();
		GetData().shaderDataBuffer->CopyTo(sizeof(GlobalShaderData), reinterpret_cast<const char*>( m_globalShaderData ));
		GetData().transformDataBuffer->CopyTo(m_relevantTransformsSize, reinterpret_cast<const char*>( m_globalTransformData ));
		GetData().previousTransformDataBuffer->CopyTo(m_relevantTransformsSize, reinterpret_cast<const char*>(m_globalPreviousTransformData));
	}
	
	std::vector<DescriptorSetLayoutBinding> PerFrameData::ProduceBindings(FrameData& frameData)
	{
		std::vector<DescriptorSetLayoutBinding> bindings = GlobalSamplers::GetInstance()->GetBindings(0);

		vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll;
	
		vk::DescriptorSetLayoutBinding& shaderDataBinding = frameData.shaderDataBinding;
		shaderDataBinding.setBinding(static_cast<uint32_t>( bindings.size() ));
		shaderDataBinding.setDescriptorCount(1);
		shaderDataBinding.setDescriptorType(DescriptorType::eUniformBuffer);
		shaderDataBinding.setStageFlags(stageFlags);
		bindings.push_back(shaderDataBinding);
		
		{
			vk::DescriptorSetLayoutBinding& transformDataBinding = frameData.transformDataBinding;
			transformDataBinding.setBinding(static_cast<uint32_t>(bindings.size()));
			transformDataBinding.setDescriptorCount(1);
			transformDataBinding.setDescriptorType(DescriptorType::eStorageBuffer);
			transformDataBinding.setStageFlags(stageFlags);
			bindings.push_back(transformDataBinding);
		}

		{
			vk::DescriptorSetLayoutBinding& transformDataBinding = frameData.previousTransformDataBinding;
			transformDataBinding.setBinding(static_cast<uint32_t>(bindings.size()));
			transformDataBinding.setDescriptorCount(1);
			transformDataBinding.setDescriptorType(DescriptorType::eStorageBuffer);
			transformDataBinding.setStageFlags(stageFlags);
			bindings.push_back(transformDataBinding);
		}
	
		return bindings;
	}
	
	std::vector<WriteDescriptorSet> PerFrameData::ProduceWrites(FrameData& frameData)
	{
		std::vector<WriteDescriptorSet> writes;
	
		WriteDescriptorSet shaderDataWrite;
		shaderDataWrite.setDescriptorCount(1);
		shaderDataWrite.setDescriptorType(frameData.shaderDataBinding.descriptorType);
		shaderDataWrite.setDstArrayElement(0);
		shaderDataWrite.setDstBinding(frameData.shaderDataBinding.binding);
		shaderDataWrite.setDstSet(frameData.m_set.GetSet());
		shaderDataWrite.setPBufferInfo(&frameData.shaderDataBuffer->GetBuffer().GetDescriptorInfo());
		writes.push_back(shaderDataWrite);
	
		{
			WriteDescriptorSet transformDataWrite;
			transformDataWrite.setDescriptorCount(1);
			transformDataWrite.setDescriptorType(frameData.transformDataBinding.descriptorType);
			transformDataWrite.setDstArrayElement(0);
			transformDataWrite.setDstBinding(frameData.transformDataBinding.binding);
			transformDataWrite.setDstSet(frameData.m_set.GetSet());
			transformDataWrite.setPBufferInfo(&frameData.transformDataBuffer->GetBuffer().GetDescriptorInfo());
			writes.push_back(transformDataWrite);
		}

		{
			WriteDescriptorSet transformDataWrite;
			transformDataWrite.setDescriptorCount(1);
			transformDataWrite.setDescriptorType(frameData.previousTransformDataBinding.descriptorType);
			transformDataWrite.setDstArrayElement(0);
			transformDataWrite.setDstBinding(frameData.previousTransformDataBinding.binding);
			transformDataWrite.setDstSet(frameData.m_set.GetSet());
			transformDataWrite.setPBufferInfo(&frameData.previousTransformDataBuffer->GetBuffer().GetDescriptorInfo());
			writes.push_back(transformDataWrite);
		}
	
		return writes;
	}
	
	void PerFrameData::GatherData()
	{
		m_globalShaderData->time = TimeManager::GetInstance()->GetTime();
		m_globalShaderData->deltaTime = TimeManager::GetInstance()->GetDeltaTime();
	
		Scene* scene = Engine::GetSceneInstance();
		CameraComponentPtr camComp = scene->GetSceneComponent<CameraComponent>();
	
		m_globalShaderData->previousWorldToView = m_globalShaderData->worldToView;
		m_globalShaderData->worldToView = camComp->CalculateViewMatrix();
		m_globalShaderData->previousViewToProj = m_globalShaderData->viewToProj;
		m_globalShaderData->viewToProj = camComp->CalculateProjectionMatrix();
		m_globalShaderData->previousCameraPos = m_globalShaderData->cameraPos;
		m_globalShaderData->cameraPos = camComp->GetParent()->transform.GetLocation();
		m_globalShaderData->previousViewVector = m_globalShaderData->viewVector;
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

