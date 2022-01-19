#include "PostProcessPass.h"
#include "../Renderer.h"
#include "data/DataManager.h"
#include "GBufferPass.h"
#include "../DataStructures.h"
#include "DeferredLightingPass.h"
#include "RTGIPass.h"
#include "LightCompositingPass.h"

namespace CGE
{
	using namespace VULKAN_HPP_NAMESPACE;

	PostProcessPass::PostProcessPass(HashString inName)
		: RenderPassBase(inName)
	{
	
	}
	
	void PostProcessPass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		VulkanSwapChain& swapChain = Engine::GetRendererInstance()->GetSwapChain();
		Image swapChainImage = swapChain.GetImage();

		auto lightingData = dataTable.GetPassData<LightCompositingPassData>();

		uint32_t frameIndex = Engine::GetFrameIndex(lightingData->frameImages.size());
		ImageMemoryBarrier attachmentBarrier = lightingData->frameImages[frameIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			AccessFlagBits::eColorAttachmentWrite,
			AccessFlagBits::eShaderRead,
			ImageAspectFlagBits::eColor,
			0, 1, 0, 1);

		std::vector<ImageMemoryBarrier> imageBarriers { attachmentBarrier };
		commandBuffer->pipelineBarrier(
			PipelineStageFlagBits::eAllCommands,
			PipelineStageFlagBits::eFragmentShader,
			DependencyFlags(),
			0, nullptr, 0, nullptr,
			static_cast<uint32_t>(imageBarriers.size()), imageBarriers.data());

		// barriers ----------------------------------------------

		MeshDataPtr meshData = MeshData::FullscreenQuad();

		PipelineData& pipelineData = executeContext.FindPipeline(m_postProcessMaterials[Engine::GetFrameIndex(m_postProcessMaterials.size())]);

		ClearValue clearValue;
		clearValue.setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));

		RenderPassBeginInfo passBeginInfo;
		passBeginInfo.setRenderPass(executeContext.GetRenderPass());
		passBeginInfo.setFramebuffer(executeContext.GetFramebuffer());
		passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), swapChain.GetExtent()));
		passBeginInfo.setClearValueCount(1);
		passBeginInfo.setPClearValues(&clearValue);

		DeviceSize offset = 0;
		commandBuffer->beginRenderPass(passBeginInfo, SubpassContents::eInline);
		commandBuffer->bindPipeline(PipelineBindPoint::eGraphics, pipelineData.pipeline);
		commandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});

		commandBuffer->bindVertexBuffers(0, 1, &meshData->GetVertexBuffer()->GetNativeBuffer(), &offset);
		commandBuffer->bindIndexBuffer(meshData->GetIndexBuffer()->GetNativeBuffer(), 0, IndexType::eUint32);
		commandBuffer->drawIndexed(meshData->GetIndexCount(), 1, 0, 0, 0);
		commandBuffer->endRenderPass();

		ImageMemoryBarrier presentBarrier;
		presentBarrier.setImage(swapChainImage);
		presentBarrier.setOldLayout(ImageLayout::eColorAttachmentOptimal);
		presentBarrier.setNewLayout(ImageLayout::ePresentSrcKHR);
		presentBarrier.subresourceRange.setAspectMask(ImageAspectFlagBits::eColor);
		presentBarrier.subresourceRange.setBaseMipLevel(0);
		presentBarrier.subresourceRange.setLevelCount(1);
		presentBarrier.subresourceRange.setBaseArrayLayer(0);
		presentBarrier.subresourceRange.setLayerCount(1);
		presentBarrier.setSrcAccessMask(AccessFlagBits::eMemoryWrite);
		presentBarrier.setDstAccessMask(AccessFlagBits::eMemoryRead);
		commandBuffer->pipelineBarrier(
			PipelineStageFlagBits::eFragmentShader,
			PipelineStageFlagBits::eHost,
			DependencyFlags(),
			0, nullptr, 0, nullptr,
			1, &presentBarrier);
	}

	void PostProcessPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		auto compositingData = dataTable.GetPassData<LightCompositingPassData>();
		m_postProcessMaterials.resize(compositingData->frameImages.size());

		for (uint32_t idx = 0; idx < compositingData->frameImages.size(); ++idx)
		{
			m_postProcessMaterials[idx] = DataManager::RequestResourceType<Material, const std::string&, const std::string&>(
				"PostProcessMaterial_" + std::to_string(idx),
				"content/shaders/PostProcessVert.spv",
				"content/shaders/PostProcessFrag.spv"
				);
			m_postProcessMaterials[idx]->SetTexture("screenImage", compositingData->frameImages[idx]);
			m_postProcessMaterials[idx]->LoadResources();
		}

		VulkanSwapChain& swapChain = Engine::GetRendererInstance()->GetSwapChain();
		initContext.SetAttachments(0, swapChain.GetTextures(), true);
	}
	
}
