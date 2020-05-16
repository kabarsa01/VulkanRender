#pragma once

#include "VulkanPassBase.h"
#include "vulkan/vulkan.hpp"
#include <array>

using namespace VULKAN_HPP_NAMESPACE;

class GBufferPass : public VulkanPassBase
{
public:
	GBufferPass(HashString inName);
	void Draw(CommandBuffer* inCmdBuffer) override;
protected:
	std::array<ClearValue, 3> clearValues;

	virtual RenderPass CreateRenderPass() override;
	virtual void CreateFramebufferResources(
		std::vector<VulkanImage>& outAttachments, 
		std::vector<ImageView>& outAttachmentViews, 
		VulkanImage& outDepthAttachment, 
		ImageView& outDepthAttachmentView, 
		uint32_t inWidth, 
		uint32_t inHeight) override;
	virtual Pipeline CreateGraphicsPipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
};
