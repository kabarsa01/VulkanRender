#pragma once

#include "VulkanPassBase.h"

class DeferredLightingPass : public VulkanPassBase
{
public:
	DeferredLightingPass(HashString inName);
	void RecordCommands(CommandBuffer* inCommandBuffer) override;
protected:
	MaterialPtr lightingMaterial;
	Texture2DPtr albedoTexture;
	Texture2DPtr normalTexture;
	Texture2DPtr depthTexture;

	virtual void OnCreate() override;
	virtual void OnDestroy() override {}
	RenderPass CreateRenderPass() override;
	void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight) override;
	void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight) override;
	Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;

};
