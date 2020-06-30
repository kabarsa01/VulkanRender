#pragma once

#include "vulkan/vulkan.hpp"
#include "VulkanPassBase.h"
#include "../resources/VulkanImage.h"
#include "../DataStructures.h"

class LightClusteringPass : public VulkanPassBase
{

public:
	MaterialPtr computeMaterial;
	VulkanImage image;
	ImageView imageView;
	Texture2DPtr texture;
	Texture2DPtr depthTexture;

	LightClusteringPass(HashString inName);
	void RecordCommands(CommandBuffer* inCommandBuffer) override;
protected:
	ClusterLightsData* clusterLightData;
	LightsList* lightsList;
	LightsIndices* lightsIndices;

	virtual void OnCreate() override;
	virtual void OnDestroy() override;
	RenderPass CreateRenderPass() override;
	void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight) override;
	void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight) override;
	Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
};
