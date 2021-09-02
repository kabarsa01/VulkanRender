#include "RTGIPass.h"


namespace CGE
{

	void RTGIPass::RecordCommands(CommandBuffer* inCommandBuffer)
	{
	}
	
	void RTGIPass::CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight)
	{
	}
	
	void RTGIPass::CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight)
	{
	}
	
	Pipeline RTGIPass::CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass)
	{
		return {};
	}
	
	RenderPass RTGIPass::CreateRenderPass()
	{
		return {};
	}
	
	void RTGIPass::OnCreate()
	{
	}
	
	void RTGIPass::OnDestroy()
	{
	}

}

