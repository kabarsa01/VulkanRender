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
	
		globalShaderData = new GlobalShaderData();
		globalTransformData = new GlobalTransformData();

		m_data.resize(2);
		
		uint32_t counter = 0;
		for (auto& frameData : m_data)
		{
			frameData.shaderDataBuffer = ObjectBase::NewObject<BufferData>(
				"PerFrameShaderData_" + std::to_string(counter), 
				sizeof(GlobalShaderData), 
				BufferUsageFlagBits::eUniformBuffer | BufferUsageFlagBits::eTransferDst, 
				true);
			frameData.shaderDataBuffer->Create();
			frameData.transformDataBuffer = ObjectBase::NewObject<BufferData>(
				"PerFrameTransformData_" + std::to_string(counter), 
				sizeof(GlobalTransformData), 
				BufferUsageFlagBits::eStorageBuffer | BufferUsageFlagBits::eTransferDst, 
				true);
			frameData.transformDataBuffer->Create();

			frameData.m_set.SetBindings(ProduceBindings(frameData));
			frameData.m_set.Create(device);

			frameData.descriptorWrites = ProduceWrites(frameData);
			device->GetDevice().updateDescriptorSets(static_cast<uint32_t>(frameData.descriptorWrites.size()), frameData.descriptorWrites.data(), 0, nullptr);

			++counter;
		}
	}
	
	void PerFrameData::Destroy()
	{
		delete globalShaderData;
		delete globalTransformData;
		
		for (auto& data : m_data)
		{
			data.shaderDataBuffer = nullptr;
			data.transformDataBuffer = nullptr;
			data.m_set.Destroy();
		}
		
		GlobalSamplers::GetInstance()->Destroy();
	}
	
	void PerFrameData::UpdateBufferData()
	{
		GatherData();
		GetData().shaderDataBuffer->CopyTo(sizeof(GlobalShaderData), reinterpret_cast<const char*>( globalShaderData ));
		GetData().transformDataBuffer->CopyTo(sizeof(GlobalTransformData), reinterpret_cast<const char*>( globalTransformData ));
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
		
		vk::DescriptorSetLayoutBinding& transformDataBinding = frameData.transformDataBinding;
		transformDataBinding.setBinding(static_cast<uint32_t>( bindings.size()) );
		transformDataBinding.setDescriptorCount(1);
		transformDataBinding.setDescriptorType(DescriptorType::eStorageBuffer);
		transformDataBinding.setStageFlags(stageFlags);
		bindings.push_back(transformDataBinding);
	
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
	
		WriteDescriptorSet transformDataWrite;
		transformDataWrite.setDescriptorCount(1);
		transformDataWrite.setDescriptorType(frameData.transformDataBinding.descriptorType);
		transformDataWrite.setDstArrayElement(0);
		transformDataWrite.setDstBinding(frameData.transformDataBinding.binding);
		transformDataWrite.setDstSet(frameData.m_set.GetSet());
		transformDataWrite.setPBufferInfo(&frameData.transformDataBuffer->GetBuffer().GetDescriptorInfo());
		writes.push_back(transformDataWrite);
	
		return writes;
	}
	
	void PerFrameData::GatherData()
	{
		globalShaderData->time = TimeManager::GetInstance()->GetTime();
		globalShaderData->deltaTime = TimeManager::GetInstance()->GetDeltaTime();
	
		Scene* scene = Engine::GetSceneInstance();
		CameraComponentPtr camComp = scene->GetSceneComponent<CameraComponent>();
	
		globalShaderData->worldToView = camComp->CalculateViewMatrix();
		globalShaderData->viewToProj = camComp->CalculateProjectionMatrix();
		globalShaderData->cameraPos = camComp->GetParent()->transform.GetLocation();
		globalShaderData->viewVector = camComp->GetParent()->transform.GetForwardVector();

		globalShaderData->numClusters = Singleton<ClusteringManager>::GetInstance()->GetNumClusters();
		globalShaderData->clusterSize = Singleton<ClusteringManager>::GetInstance()->GetClusterSize();
		globalShaderData->halfScreenOffset = Singleton<ClusteringManager>::GetInstance()->GetHalfScreenOffset();
		globalShaderData->clusterScreenOverflow = Singleton<ClusteringManager>::GetInstance()->GetClusterScreenOverflow();

		globalShaderData->cameraNear = camComp->GetNearPlane();
		globalShaderData->cameraFar = camComp->GetFarPlane();
		globalShaderData->cameraFov = camComp->GetFov();
		globalShaderData->cameraAspect = camComp->GetAspectRatio();
	
		std::memcpy(globalTransformData->modelToWorld, scene->GetModelMatrices().data(), scene->GetRelevantMatricesCount() * sizeof(glm::mat4x4));
	}
}

