#pragma once

#include "vulkan/vulkan.hpp"
#include <array>

#include "RenderPassBase.h"

namespace CGE
{

	struct GBufferPassData : public Identifiable<GBufferPassData>
	{
		std::vector<Texture2DPtr> albedos;
		std::vector<Texture2DPtr> normals;
	};

	class GBufferPass : public RenderPassBase
	{
	public:
		GBufferPass(HashString inName);
//		void RecordCommands(CommandBuffer* inCommandBuffer) override;
	protected:
		std::array<vk::ClearValue, 3> clearValues;
	
		//virtual void OnCreate() override;
		//virtual void OnDestroy() override {}
		//virtual RenderPass CreateRenderPass() override;
		//virtual void CreateColorAttachments(
		//	std::vector<VulkanImage>& outAttachments, 
		//	std::vector<ImageView>& outAttachmentViews, 
		//	uint32_t inWidth, 
		//	uint32_t inHeight) override;
		//virtual void CreateDepthAttachment(
		//	VulkanImage& outDepthAttachment,
		//	ImageView& outDepthAttachmentView,
		//	uint32_t inWidth,
		//	uint32_t inHeight) override;
		//virtual Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;

	};

}
