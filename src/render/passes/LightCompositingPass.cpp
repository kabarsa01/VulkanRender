#include "LightCompositingPass.h"
#include "utils/ResourceUtils.h"
#include "DepthPrepass.h"
#include "RTGIPass.h"
#include "data/DataManager.h"
#include "DeferredLightingPass.h"
#include "GBufferPass.h"

namespace CGE
{

	LightCompositingPass::LightCompositingPass(HashString name)
		: RenderPassBase(name)
	{
	}

	LightCompositingPass::~LightCompositingPass()
	{
	}

	void LightCompositingPass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto lightingData = dataTable.GetPassData<DeferredLightingData>();
		auto rtgiData = dataTable.GetPassData<RTGIPassData>();

		uint32_t rtIndex = Engine::GetFrameIndex(lightingData->hdrRenderTargets.size());

		//------------------------------------------------------------------------------------------------------------------------------------------------
		// image barriers 
		ImageMemoryBarrier attachmentBarrier = lightingData->hdrRenderTargets[rtIndex]->GetImage().CreateLayoutBarrier(
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		ImageMemoryBarrier giBarrier = rtgiData->lightingData[rtIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		ImageMemoryBarrier giDepthBarrier = rtgiData->giDepthData[rtIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		ImageMemoryBarrier probesTextureBarrier = rtgiData->probeGridTexture->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		ImageMemoryBarrier depthBarrier = depthData->depthTextures[rtIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eDepthAttachmentOptimal,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eDepth,
			0, 1, 0, 1);
		// buffer barriers
		vk::BufferMemoryBarrier probesBufferBarrier = rtgiData->probeGridBuffer->GetBuffer().CreateMemoryBarrier(
			0, 0, 
			vk::AccessFlagBits::eShaderWrite, 
			vk::AccessFlagBits::eShaderRead);

		std::vector<ImageMemoryBarrier> imageBarriers{ attachmentBarrier, giBarrier, giDepthBarrier, probesTextureBarrier, depthBarrier };
		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlags(),
			0, nullptr, 
			1, &probesBufferBarrier,
			static_cast<uint32_t>(imageBarriers.size()), imageBarriers.data());

		//------------------------------------------------------------------------------------------------------------------------------------------------
		// drawing

		MeshDataPtr meshData = MeshData::FullscreenQuad();

		PipelineData& pipelineData = executeContext.FindPipeline(m_materials[Engine::GetFrameIndex(m_materials.size())]);

		vk::ClearValue clearValue;
		clearValue.setColor(vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));

		vk::RenderPassBeginInfo passBeginInfo;
		passBeginInfo.setRenderPass(executeContext.GetRenderPass());
		passBeginInfo.setFramebuffer(executeContext.GetFramebuffer());
		passBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(executeContext.GetWidth(), executeContext.GetHeight())));
		passBeginInfo.setClearValueCount(1);
		passBeginInfo.setPClearValues(&clearValue);

		DeviceSize offset = 0;
		commandBuffer->beginRenderPass(passBeginInfo, vk::SubpassContents::eInline);
		commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineData.pipeline);
		commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});

		commandBuffer->bindVertexBuffers(0, 1, &meshData->GetVertexBuffer()->GetNativeBuffer(), &offset);
		commandBuffer->bindIndexBuffer(meshData->GetIndexBuffer()->GetNativeBuffer(), 0, vk::IndexType::eUint32);
		commandBuffer->drawIndexed(meshData->GetIndexCount(), 1, 0, 0, 0);
		commandBuffer->endRenderPass();
	}

	void LightCompositingPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		auto compositingData = dataTable.CreatePassData<LightCompositingPassData>();

		auto depthData = dataTable.GetPassData<DepthPrepassData>(); 
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();
		auto deferredLightingData = dataTable.GetPassData<DeferredLightingData>();
		auto rtgiData = dataTable.GetPassData<RTGIPassData>();

		compositingData->frameImages = ResourceUtils::CreateColorTextureArray("light_compositing_frame_", 2, initContext.GetWidth(), initContext.GetHeight(), vk::Format::eR16G16B16A16Unorm, false);
		initContext.SetAttachments(0, compositingData->frameImages, true);

		for (uint32_t idx = 0; idx < depthData->depthTextures.size(); ++idx)
		{
			MaterialPtr mat = DataManager::RequestResourceType<Material>(
				"compositing_pass_mat_" + std::to_string(idx),
				"content/shaders/PostProcessVert.spv",
				"content/shaders/LightCompositing.spv"
				);
			mat->SetTexture("depthTex", depthData->depthTextures[idx]);
			mat->SetTexture("normalsTex", gbufferData->normals[idx]);
			mat->SetTexture("albedoTex", gbufferData->albedos[idx]);
			mat->SetTexture("frameDirectLight", deferredLightingData->hdrRenderTargets[idx]);
			mat->SetTexture("giLight", rtgiData->lightingData[idx]);
			mat->SetTexture("probesImage", rtgiData->probeGridTexture);
			mat->SetStorageBufferExternal("probesBuffer", rtgiData->probeGridBuffer);
			mat->LoadResources();

			m_materials.push_back(mat);
		}
	}

}

