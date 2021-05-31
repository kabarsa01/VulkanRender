#ifndef __RT_SHADOW_PASS_H__
#define __RT_SHADOW_PASS_H__

#include "render/passes/VulkanPassBase.h"

namespace CGE
{

	namespace vk = VULKAN_HPP_NAMESPACE;

	class RTShadowPass : public VulkanPassBase
	{
	public:
		RTShadowPass();
		~RTShadowPass();

		void RecordCommands(CommandBuffer* inCommandBuffer) override;

	protected:
		void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight) override;
		void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight) override;
		Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
		RenderPass CreateRenderPass() override;
		void OnCreate() override;
		void OnDestroy() override;

	private:
		vk::Pipeline m_rtPipeline;
		vk::PipelineLayout m_rtPipelineLayout;
	};

}

#endif
