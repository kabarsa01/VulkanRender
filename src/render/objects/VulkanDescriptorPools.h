#pragma once

#include "vulkan/vulkan.hpp"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::DescriptorSet;
	using VULKAN_HPP_NAMESPACE::DescriptorSetLayout;
	using VULKAN_HPP_NAMESPACE::DescriptorPool;
	
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
}
