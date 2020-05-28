#pragma once

#include "vulkan/vulkan.hpp"
#include "VulkanPassBase.h"
#include "../resources/VulkanImage.h"

class LightClusteringPass : public VulkanPassBase
{

public:
	MaterialPtr computeMaterial;
	VulkanImage image;
	ImageView imageView;
	Texture2DPtr texture;

	LightClusteringPass(HashString inName);
	void RecordCommands(CommandBuffer* inCommandBuffer) override;
protected:
	void OnCreate() override;
	RenderPass CreateRenderPass() override;
	void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight) override;
	void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight) override;
	Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
};
