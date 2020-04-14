#pragma once

#include "vulkan/vulkan.hpp"
#include "../objects/VulkanDevice.h"
#include <vector>
#include "../resources/VulkanBuffer.h"
#include "../memory/DeviceMemoryManager.h"

using namespace VULKAN_HPP_NAMESPACE;

class VulkanPassBase
{
public:
	VulkanPassBase();
	virtual ~VulkanPassBase();

	void Create(VulkanDevice* inDevice);
	void Destroy();

	ImageView& GetImageView() { return imageView; }
	Image& GetImage() { return image; }

	void Draw(CommandBuffer* inCmdBuffer);
protected:
	VulkanDevice* vulkanDevice;

	RenderPass renderPass;
	PipelineLayout pipelineLayout;
	Pipeline pipeline;
	DescriptorPool descriptorPool;
	DescriptorSetLayout descriptorSetLayout;
	std::vector<DescriptorSet> descriptorSets;

	Image image;
	ImageView imageView;
	MemoryRecord imageMemRec;
	Framebuffer framebuffer;

	VulkanBuffer frameDataBuffer;
	VulkanBuffer mvpBuffer;

	uint32_t width;
	uint32_t height;

	void CreateRenderPass();
	void CreateFramebufferResources();
	void CreatePipelineLayout();
	void CreateGraphicsPipeline();
	void CreateDescriptorPool();
	void AllocateDescriptorSets();
	void UpdateDescriptorSets();
	void UpdateUniformBuffers();
};
