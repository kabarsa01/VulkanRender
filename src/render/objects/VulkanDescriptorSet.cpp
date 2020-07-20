#include "VulkanDescriptorSet.h"
#include "core/Engine.h"
#include "VulkanDevice.h"
#include "../Renderer.h"

VulkanDescriptorSet::VulkanDescriptorSet()
	: set(nullptr)
	, layout(nullptr)
{

}

VulkanDescriptorSet::~VulkanDescriptorSet()
{

}

void VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice)
{
	vulkanDevice = inVulkanDevice;
	CreateLayout();

	set = Engine::GetRendererInstance()->GetDescriptorPools().AllocateSet({layout}, pool)[0];
}

void VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice, std::vector<VulkanDescriptorSet*>& inSets)
{
	Create(inVulkanDevice, static_cast<uint32_t>(inSets.size()), inSets.data());
}

void VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice, uint32_t inCount, VulkanDescriptorSet** inSets)
{
	std::vector<DescriptorSetLayout> layouts;
	for (uint32_t index = 0; index < inCount; index++)
	{
		inSets[index]->vulkanDevice = inVulkanDevice;
		layouts.push_back(inSets[index]->CreateLayout());
	}

	DescriptorPool pool;
	std::vector<DescriptorSet> sets = Engine::GetRendererInstance()->GetDescriptorPools().AllocateSet({ layouts }, pool);

	for (uint32_t index = 0; index < inCount; index++)
	{
		inSets[index]->set = sets[index];
		inSets[index]->pool = pool;
	}
}

void VulkanDescriptorSet::Destroy()
{
	if (layout)
	{
		vulkanDevice->GetDevice().destroyDescriptorSetLayout(layout);
		layout = nullptr;
		vulkanDevice->GetDevice().freeDescriptorSets(pool, { set });
		set = nullptr;
	}
}

void VulkanDescriptorSet::SetBindings(const std::vector<DescriptorSetLayoutBinding>& inBindings)
{
	bindings = inBindings;
}

std::vector<DescriptorSetLayoutBinding> VulkanDescriptorSet::ProduceCustomBindings()
{
	return {};
}

DescriptorSetLayout& VulkanDescriptorSet::CreateLayout()
{
	if (bindings.size() == 0)
	{
		bindings = ProduceCustomBindings();
	}

	DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.setBindingCount(static_cast<uint32_t>(bindings.size()));
	layoutInfo.setPBindings(bindings.data());

	layout = vulkanDevice->GetDevice().createDescriptorSetLayout(layoutInfo);

	return layout;
}

