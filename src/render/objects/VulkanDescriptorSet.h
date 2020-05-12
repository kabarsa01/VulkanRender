#pragma once

#include "vulkan/vulkan.hpp"

using namespace VULKAN_HPP_NAMESPACE;

class VulkanDevice;

class VulkanDescriptorSet
{
public:
	VulkanDescriptorSet();
	virtual ~VulkanDescriptorSet();

	void Create(VulkanDevice* inVulkanDevice);
	static void Create(VulkanDevice* inVulkanDevice, std::vector<VulkanDescriptorSet*>& inSets);
	static void Create(VulkanDevice* inVulkanDevice, uint32_t inCount, VulkanDescriptorSet** inSets);
	void Destroy();

	DescriptorSet& GetSet() { return set; }
	DescriptorSetLayout& GetLayout() { return layout; }
	void SetBindings(const std::vector<DescriptorSetLayoutBinding>& inBindings);
	std::vector<DescriptorSetLayoutBinding>& GetBindings() { return bindings; }

	operator bool() { return set; }
protected:
	virtual std::vector<DescriptorSetLayoutBinding> ProduceCustomBindings();
private:
	VulkanDevice* vulkanDevice;

	DescriptorPool pool;
	DescriptorSet set;
	DescriptorSetLayout layout;
	std::vector<DescriptorSetLayoutBinding> bindings;

	DescriptorSetLayout& CreateLayout();
};
