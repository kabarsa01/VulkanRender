#include "VulkanDescriptorSet.h"

VulkanDescriptorSet::VulkanDescriptorSet()
{

}

VulkanDescriptorSet::~VulkanDescriptorSet()
{

}

void VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice, DescriptorPool& inDescriptorPool)
{
	vulkanDevice = inVulkanDevice;
	CreateLayout();

	DescriptorSetAllocateInfo descSetAllocInfo;
	descSetAllocInfo.setDescriptorPool(inDescriptorPool);
	descSetAllocInfo.setDescriptorSetCount(1);
	descSetAllocInfo.setPSetLayouts(&layout);

	set = vulkanDevice->GetDevice().allocateDescriptorSets(descSetAllocInfo)[0];
}

void VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice, DescriptorPool& inDescriptorPool, std::vector<VulkanDescriptorSet*>& inSets)
{
	Create(inVulkanDevice, inDescriptorPool, static_cast<uint32_t>(inSets.size()), inSets.data());
}

void VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice, DescriptorPool& inDescriptorPool, uint32_t inCount, VulkanDescriptorSet** inSets)
{
	std::vector<DescriptorSetLayout> layouts;
	for (uint32_t index = 0; index < inCount; index++)
	{
		inSets[index]->vulkanDevice = inVulkanDevice;
		layouts.push_back(inSets[index]->CreateLayout());
	}

	DescriptorSetAllocateInfo descSetAllocInfo;
	descSetAllocInfo.setDescriptorPool(inDescriptorPool);
	descSetAllocInfo.setDescriptorSetCount(static_cast<uint32_t>(layouts.size()));
	descSetAllocInfo.setPSetLayouts(layouts.data());

	std::vector<DescriptorSet> sets = inVulkanDevice->GetDevice().allocateDescriptorSets(descSetAllocInfo);
	for (uint32_t index = 0; index < inCount; index++)
	{
		inSets[index]->set = sets[index];
	}
}

void VulkanDescriptorSet::Destroy()
{
	if (layout)
	{
		vulkanDevice->GetDevice().destroyDescriptorSetLayout(layout);
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

