#include "LightClusteringPass.h"

LightClusteringPass::LightClusteringPass(HashString inName)
	: VulkanPassBase(inName)
{

}

void LightClusteringPass::Draw(CommandBuffer* inCommandBuffer)
{
}

void LightClusteringPass::OnCreate()
{
}

RenderPass LightClusteringPass::CreateRenderPass()
{
	return RenderPass();
}

void LightClusteringPass::CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight)
{
}

void LightClusteringPass::CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight)
{
}

Pipeline LightClusteringPass::CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass)
{
	ComputePipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.setLayout(inLayout);
	pipelineCreateInfo.setStage(inMaterial->GetComputeStageInfo());

	return GetVulkanDevice()->GetDevice().createComputePipeline(GetVulkanDevice()->GetPipelineCache(), pipelineCreateInfo);
}

