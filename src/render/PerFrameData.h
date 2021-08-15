#pragma once

#include "render/DataStructures.h"
#include "objects/VulkanDescriptorSet.h"
#include "resources/VulkanBuffer.h"
#include "vulkan/vulkan.hpp"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::WriteDescriptorSet;

	class PerFrameData
	{
	public:
		PerFrameData();
		virtual ~PerFrameData();
	
		void Create(VulkanDevice* inDevice);
		void Destroy();
		void UpdateBufferData();
		VulkanDescriptorSet& GetVulkanDescriptorSet() { return m_set; }
		DescriptorSet& GetSet() { return m_set.GetSet(); }
		DescriptorSetLayout& GetLayout() { return m_set.GetLayout(); }
	private:
		VulkanDevice* device;
	
		VulkanBuffer shaderDataBuffer;
		VulkanBuffer transformDataBuffer;
		DescriptorSetLayoutBinding shaderDataBinding;
		DescriptorSetLayoutBinding transformDataBinding;
	
		VulkanDescriptorSet m_set;
		std::vector<WriteDescriptorSet> descriptorWrites;
	
		GlobalShaderData* globalShaderData;
		GlobalTransformData* globalTransformData;
	
		std::vector<DescriptorSetLayoutBinding> ProduceBindings();
		std::vector<WriteDescriptorSet> ProduceWrites(VulkanDescriptorSet& inSet);
		void GatherData();
	};
}
