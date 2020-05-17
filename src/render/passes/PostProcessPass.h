#pragma once

#include "vulkan/vulkan.hpp"
#include "render/passes/VulkanPassBase.h"

class PostProcessPass : public VulkanPassBase
{
public:
	PostProcessPass(HashString inName);
	void Draw(CommandBuffer* inCommandBuffer) override;
protected:
	MaterialPtr postProcessMaterial;
	Texture2DPtr screenImage;

	void OnCreate() override;
	RenderPass CreateRenderPass() override;
	void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight) override;
	void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight) override;
	Pipeline CreateGraphicsPipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
private:
};
