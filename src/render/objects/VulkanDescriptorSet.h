#pragma once

#include "vulkan/vulkan.hpp"
#include "VulkanDevice.h"

using namespace VULKAN_HPP_NAMESPACE;

class VulkanDescriptorSet
{
public:
	VulkanDescriptorSet();
	virtual ~VulkanDescriptorSet();

	void Create(VulkanDevice* inVulkanDevice, DescriptorPool& inDescriptorPool);
	static void Create(VulkanDevice* inVulkanDevice, DescriptorPool& inDescriptorPool, std::vector<VulkanDescriptorSet*>& inSets);
	static void Create(VulkanDevice* inVulkanDevice, DescriptorPool& inDescriptorPool, uint32_t inCount, VulkanDescriptorSet** inSets);
	void Destroy();

	DescriptorSet& GetSet() { return set; }
	DescriptorSetLayout& GetLayout() { return layout; }
	void SetBindings(const std::vector<DescriptorSetLayoutBinding>& inBindings);
	std::vector<DescriptorSetLayoutBinding>& GetBindings() { return bindings; }
protected:
	virtual std::vector<DescriptorSetLayoutBinding> ProduceCustomBindings();
private:
	VulkanDevice* vulkanDevice;

	DescriptorSet set;
	DescriptorSetLayout layout;
	std::vector<DescriptorSetLayoutBinding> bindings;

	DescriptorSetLayout& CreateLayout();
};
