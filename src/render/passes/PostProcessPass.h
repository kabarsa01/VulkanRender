#pragma once

#include "vulkan/vulkan.hpp"
#include "render/passes/VulkanPassBase.h"

class PostProcessPass : public VulkanPassBase
{
public:
	PostProcessPass(HashString inName);
	void RecordCommands(CommandBuffer* inCommandBuffer) override;
protected:
	MaterialPtr postProcessMaterial;
	Texture2DPtr screenImage;

	virtual void OnCreate() override;
	virtual void OnDestroy() override {}
	RenderPass CreateRenderPass() override;
	void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight) override;
	void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight) override;
	Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
private:
};
