#include "render/passes/RTShadowPass.h"
#include "vulkan/vulkan.hpp"

namespace CGE
{

	namespace vk = VULKAN_HPP_NAMESPACE;

	void RTShadowPass::RecordCommands(CommandBuffer* inCommandBuffer)
	{
	}

	void RTShadowPass::CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight)
	{
	}

	void RTShadowPass::CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight)
	{
	}

	Pipeline RTShadowPass::CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass)
	{
		VulkanDevice* device = GetVulkanDevice();
		vk::Device& nativeDevice = device->GetDevice();

		vk::PipelineCacheCreateInfo pipelineCacheInfo;
		vk::PipelineCache pipelineCache = nativeDevice.createPipelineCache(pipelineCacheInfo);

		auto func = PFN_vkCreateDeferredOperationKHR(vkGetDeviceProcAddr(nativeDevice, "createDeferredOperationKHR"));
		vk::DeferredOperationKHR deferredOperation;

		//------------------------------------------------------------------
		// shader stages and groups

		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

		// raygen group
		vk::RayTracingShaderGroupCreateInfoKHR groupInfo;
		groupInfo.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral);
		groupInfo.setAnyHitShader(VK_SHADER_UNUSED_KHR);
		groupInfo.setClosestHitShader(VK_SHADER_UNUSED_KHR);
		groupInfo.setIntersectionShader(VK_SHADER_UNUSED_KHR);
		groupInfo.setGeneralShader(static_cast<uint32_t>(shaderStages.size()));
		shaderGroups.push_back(groupInfo);

		vk::PipelineShaderStageCreateInfo shaderStageInfo;
		shaderStageInfo.setStage(vk::ShaderStageFlagBits::eRaygenKHR);
		shaderStageInfo.setFlags(vk::PipelineShaderStageCreateFlags{});
		shaderStageInfo.setPName("main"); // temp name
		//shaderStageInfo.setModule()

		//------------------------------------------------------------------
		// pipeline create info

		vk::RayTracingPipelineCreateInfoKHR pipelineInfo;
		pipelineInfo.setPGroups(nullptr);
		pipelineInfo.setPStages(nullptr);

		auto result = nativeDevice.createRayTracingPipelineKHR(deferredOperation, pipelineCache, pipelineInfo);
		if (result.result != vk::Result::eSuccess)
		{
			return Pipeline();
		}

		return result.value;
	}

	RenderPass RTShadowPass::CreateRenderPass()
	{
		return RenderPass();
	}

	void RTShadowPass::OnCreate()
	{
	}

	void RTShadowPass::OnDestroy()
	{
	}

}

