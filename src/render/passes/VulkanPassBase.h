#pragma once

#include "vulkan/vulkan.hpp"
#include "../objects/VulkanDevice.h"
#include <vector>
#include "../resources/VulkanBuffer.h"
#include "../memory/DeviceMemoryManager.h"
#include "../resources/VulkanImage.h"
#include "../PipelineStorage.h"
#include "data/Material.h"

using namespace VULKAN_HPP_NAMESPACE;

class VulkanPassBase
{
public:
	VulkanPassBase(HashString inName = HashString::NONE);
	virtual ~VulkanPassBase();

	void Create(VulkanDevice* inDevice);
	void Destroy();

	ImageView& GetImageView() { return colorAttachmentImageView; }
	Image& GetImage() { return colorAttachmentImage.GetImage(); }

	void Draw(CommandBuffer* inCmdBuffer);
protected:
	HashString name;
	VulkanDevice* vulkanDevice;

	RenderPass renderPass;
	PipelineLayout pipelineLayout;
//	Pipeline pipeline;
	DescriptorPool descriptorPool;
	DescriptorSetLayout descriptorSetLayout;
	std::vector<DescriptorSet> descriptorSets;
	PipelineStorage pipelineStorage;

	VulkanImage colorAttachmentImage;
	ImageView colorAttachmentImageView;
	Framebuffer framebuffer;

	VulkanBuffer frameDataBuffer;
	VulkanBuffer mvpBuffer;

	uint32_t width;
	uint32_t height;

	void CreateRenderPass();
	void CreateFramebufferResources();
	void CreatePipelineLayout();
	Pipeline FindGraphicsPipeline(MaterialPtr inMaterial);
	void CreateDescriptorPool();
	void AllocateDescriptorSets();
	void UpdateDescriptorSets();
	void UpdateUniformBuffers();

	void CreateTextures();

	Pipeline CreateGraphicsPipeline(MaterialPtr inMaterial);
};
