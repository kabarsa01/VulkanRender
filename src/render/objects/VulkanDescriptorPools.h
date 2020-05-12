#pragma once

#include "vulkan/vulkan.hpp"

using namespace VULKAN_HPP_NAMESPACE;

class VulkanDevice;

class VulkanDescriptorPools
{
public:
	VulkanDescriptorPools();
	virtual ~VulkanDescriptorPools();

	void Create(VulkanDevice* inVulkanDevice);
	void Destroy();

	std::vector<DescriptorSet> AllocateSet(const std::vector<DescriptorSetLayout>& inLayouts, DescriptorPool& outPool);
private:
	VulkanDevice* vulkanDevice;
	std::vector<DescriptorPool> pools;

	DescriptorPool ConstructDescriptorPool();
};
