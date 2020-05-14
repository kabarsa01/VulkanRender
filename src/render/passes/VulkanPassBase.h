#pragma once

#include "vulkan/vulkan.hpp"
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

	std::vector<VulkanImage> GetAttachments() { return attachments; }
	std::vector<ImageView> GetAttachmentViews() { return attachmentViews; }

	void Draw(CommandBuffer* inCmdBuffer);
protected:
	HashString name;
	class VulkanDevice* vulkanDevice;
	class Renderer* renderer;

	RenderPass renderPass;
	Framebuffer framebuffer;
	std::vector<VulkanImage> attachments;
	std::vector<ImageView> attachmentViews;

	uint32_t width;
	uint32_t height;

	virtual RenderPass CreateRenderPass();
	virtual void CreateFramebufferResources(
		std::vector<VulkanImage>& outAttachments, 
		std::vector<ImageView>& outAttachmentViews,
		uint32_t inWidth,
		uint32_t inHeight);


	Framebuffer CreateFramebuffer(
		RenderPass inRenderPass,
		std::vector<ImageView>& inAttachmentViews,
		uint32_t inWidth,
		uint32_t inHeight);
	PipelineData& FindGraphicsPipeline(MaterialPtr inMaterial);
	Pipeline CreateGraphicsPipeline(MaterialPtr inMaterial, PipelineLayout inLayout);
	PipelineLayout CreatePipelineLayout(std::vector<DescriptorSetLayout>& inDescriptorSetLayouts);
	DescriptorSetLayout CreateDescriptorSetLayout(MaterialPtr inMaterial);
};
