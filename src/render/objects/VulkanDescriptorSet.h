#pragma once

#include "vulkan/vulkan.hpp"
#include "../shader/Shader.h"
#include "../shader/RtShader.h"

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
		static std::vector<VulkanDescriptorSet> Create(VulkanDevice* inVulkanDevice, const std::vector<ShaderPtr>& inShaders);
		static VulkanDescriptorSet Create(VulkanDevice* inVulkanDevice, const std::vector<ShaderPtr>& inShaders, uint32_t inSetIndex);
		static std::vector<VulkanDescriptorSet> Create(VulkanDevice* inVulkanDevice, const std::vector<RtShaderPtr>& inShaders);
		static VulkanDescriptorSet Create(VulkanDevice* inVulkanDevice, const std::vector<RtShaderPtr>& inShaders, uint32_t inSetIndex);
		void Update(std::vector<vk::WriteDescriptorSet>& writes);
		void Destroy();
	
		DescriptorSet& GetSet() { return m_set; }
		DescriptorSetLayout& GetLayout() { return m_layout; }
		void SetBindings(const std::vector<DescriptorSetLayoutBinding>& inBindings);
		std::vector<DescriptorSetLayoutBinding>& GetBindings() { return m_bindings; }
		void AddBinding(const DescriptorSetLayoutBinding& inBinding);
	
		operator bool() { return m_set; }
	protected:
		virtual std::vector<DescriptorSetLayoutBinding> ProduceCustomBindings();
	private:
		VulkanDevice* m_vulkanDevice;
	
		DescriptorPool m_pool;
		DescriptorSet m_set;
		DescriptorSetLayout m_layout;
		std::vector<DescriptorSetLayoutBinding> m_bindings;
	
		DescriptorSetLayout& CreateLayout();
	};
}
