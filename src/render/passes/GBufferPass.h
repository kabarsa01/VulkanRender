#pragma once

#include "VulkanPassBase.h"
#include "vulkan/vulkan.hpp"
#include <array>

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::ClearValue;
	using VULKAN_HPP_NAMESPACE::CommandBuffer;
	using VULKAN_HPP_NAMESPACE::RenderPass;
	using VULKAN_HPP_NAMESPACE::ImageView;
	using VULKAN_HPP_NAMESPACE::Pipeline;
	using VULKAN_HPP_NAMESPACE::PipelineLayout;

	class GBufferPass : public VulkanPassBase
	{
	public:
		GBufferPass(HashString inName);
		void RecordCommands(CommandBuffer* inCommandBuffer) override;
	protected:
		std::array<ClearValue, 3> clearValues;
	
		virtual void OnCreate() override;
		virtual void OnDestroy() override {}
		virtual RenderPass CreateRenderPass() override;
		virtual void CreateColorAttachments(
			std::vector<VulkanImage>& outAttachments, 
			std::vector<ImageView>& outAttachmentViews, 
			uint32_t inWidth, 
			uint32_t inHeight) override;
		virtual void CreateDepthAttachment(
			VulkanImage& outDepthAttachment,
			ImageView& outDepthAttachmentView,
			uint32_t inWidth,
			uint32_t inHeight) override;
		virtual Pipeline CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass) override;
	};
}
