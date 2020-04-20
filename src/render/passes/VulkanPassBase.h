#pragma once

#include "vulkan/vulkan.hpp"
#include "../objects/VulkanDevice.h"
#include <vector>
#include "../resources/VulkanBuffer.h"
#include "../memory/DeviceMemoryManager.h"
#include "../resources/VulkanImage.h"

using namespace VULKAN_HPP_NAMESPACE;

class VulkanPassBase
{
public:
	VulkanPassBase();
	virtual ~VulkanPassBase();

	void Create(VulkanDevice* inDevice);
	void Destroy();

	ImageView& GetImageView() { return colorAttachmentImageView; }
	Image& GetImage() { return colorAttachmentImage.GetImage(); }

	void Draw(CommandBuffer* inCmdBuffer);
protected:
	VulkanDevice* vulkanDevice;

	RenderPass renderPass;
	PipelineLayout pipelineLayout;
	Pipeline pipeline;
	DescriptorPool descriptorPool;
	DescriptorSetLayout descriptorSetLayout;
	std::vector<DescriptorSet> descriptorSets;

	VulkanImage colorAttachmentImage;
	ImageView colorAttachmentImageView;
	Framebuffer framebuffer;

	VulkanBuffer frameDataBuffer;
	VulkanBuffer mvpBuffer;

	VulkanImage diffuseTexture;

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

	void CreateTextures();
};
