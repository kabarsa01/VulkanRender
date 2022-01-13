#include "DeferredLightingPass.h"
#include "utils/ResourceUtils.h"
#include "data/MeshData.h"
#include "GBufferPass.h"
#include "data/DataManager.h"
#include "../DataStructures.h"
#include "../RtScene.h"
#include "utils/Singleton.h"
#include "RTShadowPass.h"
#include "core/Engine.h"
#include "ClusterComputePass.h"
#include "DepthPrepass.h"
#include "RTGIPass.h"

namespace CGE
{
	using namespace VULKAN_HPP_NAMESPACE;

	DeferredLightingPass::DeferredLightingPass(HashString inName)
		: RenderPassBase(inName)
	{
	}
	
	void DeferredLightingPass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto clusterData = dataTable.GetPassData<ClusterComputeData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();
		auto rtShadowsData = dataTable.GetPassData<RTShadowsData>();
//		auto rtGIData = dataTable.GetPassData<RTGIPassData>();

		uint32_t frameIndex = Engine::GetFrameIndex(m_lightingMaterials.size());
		MaterialPtr lightingMat = m_lightingMaterials[frameIndex];
		BufferDataPtr buffer = clusterData->clusterLightsData;
		TextureDataPtr depthTex = lightingMat->GetSampledTexture("depthTex");

		auto textures = lightingMat->GetSampledTextures("albedoTex", "normalsTex");

		RTShadowPass* rtPass = Engine::GetRendererInstance()->GetRTShadowPass();
	
		// barriers ----------------------------------------------
		BufferMemoryBarrier clusterDataBarrier = buffer->GetBuffer().CreateMemoryBarrier(
			0, 0, 
			AccessFlagBits::eShaderWrite, 
			AccessFlagBits::eShaderRead);
		ImageMemoryBarrier depthTextureBarrier = depthTex->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			AccessFlagBits::eShaderWrite,
			AccessFlagBits::eShaderRead,
			ImageAspectFlagBits::eDepth | ImageAspectFlagBits::eStencil,
			0, 1, 0, 1);
		ImageMemoryBarrier visibilityTextureBarrier = rtShadowsData->visibilityTex->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			AccessFlagBits::eShaderWrite,
			AccessFlagBits::eShaderRead,
			ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		std::vector<ImageMemoryBarrier> barriers{ depthTextureBarrier, visibilityTextureBarrier };
		for (auto tex : rtShadowsData->visibilityTextures)
		{
			ImageMemoryBarrier visibilityBarrier = tex->GetImage().CreateLayoutBarrier(
				ImageLayout::eUndefined,
				ImageLayout::eShaderReadOnlyOptimal,
				AccessFlagBits::eShaderWrite,
				AccessFlagBits::eShaderRead,
				ImageAspectFlagBits::eColor,
				0, 1, 0, 1);
			barriers.push_back(visibilityBarrier);
		}
		//ImageMemoryBarrier giLightingData = rtGIData->lightingData[Engine::GetFrameIndex(rtGIData->lightingData.size())]->GetImage().CreateLayoutBarrier(
		//	ImageLayout::eUndefined,
		//	ImageLayout::eShaderReadOnlyOptimal,
		//	AccessFlagBits::eShaderWrite,
		//	AccessFlagBits::eShaderRead,
		//	ImageAspectFlagBits::eColor,
		//	0, 1, 0, 1);
		//barriers.push_back(giLightingData);

		commandBuffer->pipelineBarrier(
			PipelineStageFlagBits::eAllGraphics | PipelineStageFlagBits::eRayTracingShaderKHR,
			PipelineStageFlagBits::eAllGraphics,
			DependencyFlags(),
			0, nullptr,
			1, &clusterDataBarrier,
			static_cast<uint32_t>(barriers.size()), barriers.data());
	
		MeshDataPtr meshData = MeshData::FullscreenQuad();
		PipelineData& pipelineData = executeContext.FindPipeline(lightingMat);
	
		ClearValue clearValue;
		clearValue.setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));
	
		RenderPassBeginInfo passBeginInfo;
		passBeginInfo.setRenderPass(executeContext.GetRenderPass());
		passBeginInfo.setFramebuffer(executeContext.GetFramebuffer());
		passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), Extent2D(executeContext.GetWidth(), executeContext.GetHeight()) ));
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
	}

	void DeferredLightingPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto clusteringData = dataTable.GetPassData<ClusterComputeData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();
		auto rtShadowsData = dataTable.GetPassData<RTShadowsData>();
		//auto rtGIData = dataTable.GetPassData<RTGIPassData>();

		bool isValid = (depthData->depthTextures.size() == clusteringData->lightsList.size()) && (clusteringData->lightsList.size() == gbufferData->albedos.size());
		assert(isValid && "Number of render targets mismatch");

		for (uint32_t idx = 0; idx < depthData->depthTextures.size(); ++idx)
		{
			MaterialPtr lightingMaterial = DataManager::RequestResourceType<Material, const std::string&, const std::string&>(
				"DeferredLightingMaterial_" + std::to_string(idx),
				"content/shaders/ScreenSpaceVert.spv",
				"content/shaders/DeferredLighting.spv"
				);
			lightingMaterial->SetStorageBufferExternal("clusterLightsData", clusteringData->clusterLightsData);
			lightingMaterial->SetUniformBufferExternal("lightsList", clusteringData->lightsList[idx]);
			lightingMaterial->SetUniformBufferExternal("lightsIndices", clusteringData->lightsIndices[idx]);
			lightingMaterial->SetTexture("albedoTex", gbufferData->albedos[idx]);
			lightingMaterial->SetTexture("normalsTex", gbufferData->normals[idx]);
			lightingMaterial->SetTexture("depthTex", depthData->depthTextures[idx]);
			lightingMaterial->SetTextureArray("visibilityTextures", rtShadowsData->visibilityTextures);
			lightingMaterial->SetAccelerationStructure("topLevelAS", Singleton<RtScene>::GetInstance()->GetTlas().accelerationStructure);
			lightingMaterial->LoadResources();

			m_lightingMaterials.push_back(lightingMaterial);
		}

		auto deferredLightingData = dataTable.CreatePassData<DeferredLightingData>();
		deferredLightingData->hdrRenderTargets = ResourceUtils::CreateColorTextureArray("DeferredLightingRT", 2, initContext.GetWidth(), initContext.GetHeight(), vk::Format::eR16G16B16A16Sfloat, false);
		initContext.SetAttachments(0, deferredLightingData->hdrRenderTargets, true);
	}

}
