#ifndef __RT_GI_PASS_H__
#define __RT_GI_PASS_H__

#include <vulkan/vulkan.hpp>

#include "render/passes/VulkanPassBase.h"
#include "messages/MessageSubscriber.h"

namespace CGE
{

	class RTGIPass : public VulkanPassBase
	{
	public:
		void RecordCommands(CommandBuffer* inCommandBuffer) override;
	protected:
		void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight) override;
		void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight) override;
	
		vk::Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
		vk::RenderPass CreateRenderPass() override;
	
		void OnCreate() override;
		void OnDestroy() override;
	private:
	};

}

#endif
