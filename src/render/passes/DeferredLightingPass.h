#pragma once

#include "RenderPassBase.h"
#include "data/Material.h"

namespace CGE
{

	struct DeferredLightingData : public Identifiable<DeferredLightingData>
	{
		std::vector<Texture2DPtr> hdrRenderTarget;
	};

	class DeferredLightingPass : public RenderPassBase
	{
	public:
		DeferredLightingPass(HashString inName);
		//void RecordCommands(CommandBuffer* inCommandBuffer) override;
	protected:
		std::vector<MaterialPtr> m_lightingMaterials;
		//Texture2DPtr albedoTexture;
		//Texture2DPtr normalTexture;
		//Texture2DPtr depthTexture;
		//Texture2DPtr visibilityTexture;
	
		//virtual void OnCreate() override;
		//virtual void OnDestroy() override {}
		//RenderPass CreateRenderPass() override;
		//void CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight) override;
		//void CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight) override;
		//Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
	

		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;
	};
}
