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
	
		shaderDataBuffer.createInfo.setSharingMode(SharingMode::eExclusive);
		shaderDataBuffer.createInfo.setSize(sizeof(GlobalShaderData));
		shaderDataBuffer.createInfo.setUsage(BufferUsageFlagBits::eUniformBuffer | BufferUsageFlagBits::eTransferDst);
		shaderDataBuffer.Create(device);
		shaderDataBuffer.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
		shaderDataBuffer.CreateStagingBuffer();
	
		transformDataBuffer.createInfo.setSharingMode(SharingMode::eExclusive);
		transformDataBuffer.createInfo.setSize(sizeof(GlobalTransformData));
		transformDataBuffer.createInfo.setUsage(BufferUsageFlagBits::eStorageBuffer | BufferUsageFlagBits::eTransferDst);
		transformDataBuffer.Create(device);
		transformDataBuffer.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
		transformDataBuffer.CreateStagingBuffer();
	
		set.SetBindings(ProduceBindings());
		set.Create(device);
	
		descriptorWrites = ProduceWrites(set);
		device->GetDevice().updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
	
	void PerFrameData::Destroy()
	{
		delete globalShaderData;
		delete globalTransformData;
	
		shaderDataBuffer.Destroy();
		transformDataBuffer.Destroy();
		set.Destroy();
		GlobalSamplers::GetInstance()->Destroy();
	}
	
	void PerFrameData::UpdateBufferData()
	{
		GatherData();
		shaderDataBuffer.CopyTo(sizeof(GlobalShaderData), reinterpret_cast<const char*>( globalShaderData ), true);
		transformDataBuffer.CopyTo(sizeof(GlobalTransformData), reinterpret_cast<const char*>( globalTransformData ), true);
	}
	
	std::vector<DescriptorSetLayoutBinding> PerFrameData::ProduceBindings()
	{
		std::vector<DescriptorSetLayoutBinding> bindings = GlobalSamplers::GetInstance()->GetBindings(0);

		vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll;
	
		shaderDataBinding.setBinding(static_cast<uint32_t>( bindings.size() ));
		shaderDataBinding.setDescriptorCount(1);
		shaderDataBinding.setDescriptorType(DescriptorType::eUniformBuffer);
		shaderDataBinding.setStageFlags(stageFlags);
		bindings.push_back(shaderDataBinding);
	
		transformDataBinding.setBinding(static_cast<uint32_t>( bindings.size()) );
		transformDataBinding.setDescriptorCount(1);
		transformDataBinding.setDescriptorType(DescriptorType::eStorageBuffer);
		transformDataBinding.setStageFlags(stageFlags);
		bindings.push_back(transformDataBinding);
	
		return bindings;
	}
	
	std::vector<WriteDescriptorSet> PerFrameData::ProduceWrites(VulkanDescriptorSet& inSet)
	{
		std::vector<WriteDescriptorSet> writes;
	
		WriteDescriptorSet shaderDataWrite;
		shaderDataWrite.setDescriptorCount(1);
		shaderDataWrite.setDescriptorType(shaderDataBinding.descriptorType);
		shaderDataWrite.setDstArrayElement(0);
		shaderDataWrite.setDstBinding(shaderDataBinding.binding);
		shaderDataWrite.setDstSet(inSet.GetSet());
		shaderDataWrite.setPBufferInfo(&shaderDataBuffer.GetDescriptorInfo());
		writes.push_back(shaderDataWrite);
	
		WriteDescriptorSet transformDataWrite;
		transformDataWrite.setDescriptorCount(1);
		transformDataWrite.setDescriptorType(transformDataBinding.descriptorType);
		transformDataWrite.setDstArrayElement(0);
		transformDataWrite.setDstBinding(transformDataBinding.binding);
		transformDataWrite.setDstSet(inSet.GetSet());
		transformDataWrite.setPBufferInfo(&transformDataBuffer.GetDescriptorInfo());
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
		globalShaderData->cameraNear = camComp->GetNearPlane();
		globalShaderData->cameraFar = camComp->GetFarPlane();
		globalShaderData->cameraFov = camComp->GetFov();
		globalShaderData->cameraAspect = camComp->GetAspectRatio();
	
		std::memcpy(globalTransformData->modelToWorld, scene->GetModelMatrices().data(), scene->GetRelevantMatricesCount() * sizeof(glm::mat4x4));
	}
}

