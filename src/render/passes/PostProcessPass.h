#pragma once

#include "vulkan/vulkan.hpp"
#include "RenderPassBase.h"

namespace CGE
{
	class PostProcessPass : public RenderPassBase
	{
	public:
		PostProcessPass(HashString inName);
//		void RecordCommands(CommandBuffer* inCommandBuffer) override;
	protected:
		std::vector<MaterialPtr> m_postProcessMaterials;
		std::vector<Texture2DPtr> m_screenImages;
	

		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;

		//virtual void OnCreate() override;
		//virtual void OnDestroy() override {}
		//RenderPass CreateRenderPass() override;
		//void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight) override;
		//void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight) override;
		//Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
	private:
	};
}
