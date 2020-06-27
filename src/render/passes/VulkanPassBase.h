#pragma once

#include "vulkan/vulkan.hpp"
#include <vector>
#include "../resources/VulkanBuffer.h"
#include "../memory/DeviceMemoryManager.h"
#include "../resources/VulkanImage.h"
#include "../PipelineRegistry.h"
#include "data/Material.h"

using namespace VULKAN_HPP_NAMESPACE;

class VulkanDevice;
class Renderer;

class VulkanPassBase
{
public:
	VulkanPassBase(HashString inName);
	virtual ~VulkanPassBase();

	void Create();
	void Destroy();

	void SetResolution(uint32_t inWidth, uint32_t inHeight);

	inline HashString& GetName() { return name; }
	inline RenderPass& GetRenderPass() { return renderPass; }
	inline Framebuffer& GetFramebuffer() { return framebuffer; }
	inline const std::vector<VulkanImage>& GetAttachments() { return attachments; }
	inline const std::vector<ImageView>& GetAttachmentViews() { return attachmentViews; }
	inline VulkanImage& GetDepthAttachment() { return depthAttachment; }
	inline ImageView& GetDepthAttachmentView() { return depthAttachmentView; }
	inline uint32_t GetWidth() { return width; }
	inline uint32_t GetHeight() { return height; }

	void SetExternalDepth(const VulkanImage& inDepthAttachment, const ImageView& inDepthAttachmentView);

	virtual void RecordCommands(CommandBuffer* inCommandBuffer) = 0;
protected:
	inline VulkanDevice* GetVulkanDevice() { return vulkanDevice; }
	inline Renderer* GetRenderer() { return renderer; }

	virtual void OnCreate() = 0;
	virtual void OnDestroy() = 0;
	virtual RenderPass CreateRenderPass() = 0;
	virtual void CreateColorAttachments(
		std::vector<VulkanImage>& outAttachments, 
		std::vector<ImageView>& outAttachmentViews,
		uint32_t inWidth,
		uint32_t inHeight) = 0;
	virtual void CreateDepthAttachment(
		VulkanImage& outDepthAttachment,
		ImageView& outDepthAttachmentView,
		uint32_t inWidth,
		uint32_t inHeight) = 0;
	virtual Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) = 0;


	Framebuffer CreateFramebuffer(
		RenderPass inRenderPass,
		std::vector<ImageView>& inAttachmentViews,
		uint32_t inWidth,
		uint32_t inHeight);
	PipelineData& FindPipeline(MaterialPtr inMaterial);
	PipelineLayout CreatePipelineLayout(std::vector<DescriptorSetLayout>& inDescriptorSetLayouts);
	DescriptorSetLayout CreateDescriptorSetLayout(MaterialPtr inMaterial);
private:
	HashString name;
	VulkanDevice* vulkanDevice;
	Renderer* renderer;

	RenderPass renderPass;
	Framebuffer framebuffer;
	std::vector<VulkanImage> attachments;
	std::vector<ImageView> attachmentViews;
	VulkanImage depthAttachment;
	ImageView depthAttachmentView;

	uint32_t width = 1280;
	uint32_t height = 720;
	bool isDepthExternal;

	VulkanPassBase() {}
};
