#include "PerFrameData.h"
#include "objects/VulkanDevice.h"
#include "GlobalSamplers.h"
#include "scene/Scene.h"
#include "core/Engine.h"
#include "scene/camera/CameraComponent.h"
#include "core/TimeManager.h"

PerFrameData::PerFrameData()
{

}

PerFrameData::~PerFrameData()
{

}

void PerFrameData::Create(VulkanDevice* inDevice)
{
	device = inDevice;

	shaderDataBuffer.createInfo.setSharingMode(SharingMode::eExclusive);
	shaderDataBuffer.createInfo.setSize(sizeof(ShaderGlobalData));
	// lets settle for host visible for now, so no transfer dst at the moment
	// will handle later, using large common buffer
	shaderDataBuffer.createInfo.setUsage(BufferUsageFlagBits::eUniformBuffer); 
	shaderDataBuffer.Create(device);
	shaderDataBuffer.BindMemory(MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent);

	set.SetBindings(ProduceBindings());
	set.Create(device);

	descriptorWrites = ProduceWrites(set);
	device->GetDevice().updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void PerFrameData::Destroy()
{
	if (shaderDataBuffer)
	{
		shaderDataBuffer.Destroy();
	}
	set.Destroy();
	GlobalSamplers::GetInstance()->Destroy();
}

void PerFrameData::UpdateBufferData()
{
	GatherData();
	shaderDataBuffer.CopyTo(sizeof(ShaderGlobalData), reinterpret_cast<const char*>( &shaderGlobalData ));
}

std::vector<DescriptorSetLayoutBinding> PerFrameData::ProduceBindings()
{
	GlobalSamplers::GetInstance()->Create(device);
	std::vector<DescriptorSetLayoutBinding> bindings = GlobalSamplers::GetInstance()->GetBindings(0);

	shaderGlobalDataBinding.setBinding(static_cast<uint32_t>( bindings.size() ));
	shaderGlobalDataBinding.setDescriptorCount(1);
	shaderGlobalDataBinding.setDescriptorType(DescriptorType::eUniformBuffer);
	shaderGlobalDataBinding.setStageFlags(ShaderStageFlagBits::eAllGraphics);

	bindings.push_back(shaderGlobalDataBinding);

	return bindings;
}

std::vector<WriteDescriptorSet> PerFrameData::ProduceWrites(VulkanDescriptorSet& inSet)
{
	std::vector<WriteDescriptorSet> writes;

	WriteDescriptorSet write;
	write.setDescriptorCount(1);
	write.setDescriptorType(shaderGlobalDataBinding.descriptorType);
	write.setDstArrayElement(0);
	write.setDstBinding(shaderGlobalDataBinding.binding);
	write.setDstSet(inSet.GetSet());
	write.setPBufferInfo(&shaderDataBuffer.GetDescriptorInfo());

	writes.push_back(write);

	return writes;
}

void PerFrameData::GatherData()
{
	shaderGlobalData.time = TimeManager::GetInstance()->GetTime();
	shaderGlobalData.deltaTime = TimeManager::GetInstance()->GetDeltaTime();

	ScenePtr scene = Engine::GetSceneInstance();
	CameraComponentPtr camComp = scene->GetSceneComponent<CameraComponent>();
	shaderGlobalData.view = camComp->CalculateViewMatrix();
	shaderGlobalData.proj = camComp->CalculateProjectionMatrix();
}

