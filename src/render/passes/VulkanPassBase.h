#pragma once

#include "vulkan/vulkan.hpp"
#include "../objects/VulkanDevice.h"
#include <vector>
#include "../resources/VulkanBuffer.h"
#include "../memory/DeviceMemoryManager.h"
#include "../resources/VulkanImage.h"
#include "../PipelineRegistry.h"
#include "data/Material.h"

using namespace VULKAN_HPP_NAMESPACE;

class VulkanPassBase
{
public:
	VulkanPassBase(HashString inName = HashString::NONE);
	virtual ~VulkanPassBase();

	void Create();
	void Destroy();

	ImageView& GetImageView() { return colorAttachmentImageView; }
	Image& GetImage() { return colorAttachmentImage.GetImage(); }

	void Draw(CommandBuffer* inCmdBuffer);
protected:
	HashString name;
	VulkanDevice* vulkanDevice;
	class Renderer* renderer;

	RenderPass renderPass;

	VulkanImage colorAttachmentImage;
	ImageView colorAttachmentImageView;
	Framebuffer framebuffer;

	uint32_t width;
	uint32_t height;

	void CreateRenderPass();
	void CreateFramebufferResources();
	PipelineData& FindGraphicsPipeline(MaterialPtr inMaterial);
	Pipeline CreateGraphicsPipeline(MaterialPtr inMaterial, PipelineLayout inLayout);
	PipelineLayout CreatePipelineLayout(std::vector<DescriptorSetLayout>& inDescriptorSetLayouts);
	DescriptorSetLayout CreateDescriptorSetLayout(MaterialPtr inMaterial);
};
