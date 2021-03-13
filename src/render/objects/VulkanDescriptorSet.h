#pragma once

#include "vulkan/vulkan.hpp"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::DescriptorSet;
	using VULKAN_HPP_NAMESPACE::DescriptorSetLayout;
	using VULKAN_HPP_NAMESPACE::DescriptorSetLayoutBinding;
	using VULKAN_HPP_NAMESPACE::DescriptorPool;
	
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
}
