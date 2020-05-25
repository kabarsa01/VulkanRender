#pragma once

#include "VulkanPassBase.h"

class LightClusteringPass : public VulkanPassBase
{

public:
	LightClusteringPass(HashString inName);
	void Draw(CommandBuffer* inCommandBuffer) override;
protected:
	void OnCreate() override;
	RenderPass CreateRenderPass() override;
	void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight) override;
	void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight) override;
	Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;

};
