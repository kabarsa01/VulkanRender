#pragma once

#include "render/DataStructures.h"
#include "objects/VulkanDescriptorSet.h"
#include "resources/VulkanBuffer.h"
#include "vulkan/vulkan.hpp"

using namespace VULKAN_HPP_NAMESPACE;

class PerFrameData
{
public:
	PerFrameData();
	virtual ~PerFrameData();

	void Create(VulkanDevice* inDevice);
	void Destroy();
	void UpdateBufferData();
	VulkanDescriptorSet& GetVulkanDescriptorSet() { return set; }
	DescriptorSet& GetSet() { return set.GetSet(); }
	DescriptorSetLayout& GetLayout() { return set.GetLayout(); }
private:
	VulkanDevice* device;

	VulkanBuffer shaderDataBuffer;
	VulkanBuffer transformDataBuffer;
	DescriptorSetLayoutBinding shaderDataBinding;
	DescriptorSetLayoutBinding transformDataBinding;

	VulkanDescriptorSet set;
	std::vector<WriteDescriptorSet> descriptorWrites;

	GlobalShaderData* globalShaderData;
	GlobalTransformData* globalTransformData;

	std::vector<DescriptorSetLayoutBinding> ProduceBindings();
	std::vector<WriteDescriptorSet> ProduceWrites(VulkanDescriptorSet& inSet);
	void GatherData();
};
