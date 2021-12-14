#include "render/passes/RTShadowPass.h"
#include "vulkan/vulkan.hpp"
#include "../RtScene.h"
#include "utils/Singleton.h"
#include "utils/ResourceUtils.h"
#include "GBufferPass.h"
#include "../shader/ShaderResourceMapper.h"
#include "../PerFrameData.h"
#include "../objects/VulkanDevice.h"
#include "../objects/VulkanPhysicalDevice.h"
#include "data/DataManager.h"
#include "core/Engine.h"
#include "../Renderer.h"
#include "DepthPrepass.h"
#include "ClusterComputePass.h"


namespace CGE
{

	namespace vk = VULKAN_HPP_NAMESPACE;

	RTShadowPass::RTShadowPass(HashString name)
		: RenderPassBase(name)
	{
//		m_subscriber.AddHandler<GlobalPreFrameMessage>(this, &RTShadowPass::HandlePreUpdate);
	}

	RTShadowPass::~RTShadowPass()
	{
//		m_subscriber.UnregisterHandlers();

		vk::Device& device = Engine::GetRendererInstance()->GetDevice();
		// TODO cleanup
		for (auto data : m_frameDataArray)
		{
			device.destroyPipeline(data.m_rtPipeline);
			device.destroyPipelineLayout(data.m_rtPipelineLayout);
		}
	}

	void RTShadowPass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		RtScene* rtScene = Singleton<RtScene>::GetInstance();

		RtShadowPassFrameData& frameData = m_frameDataArray[Engine::GetFrameIndex(m_frameDataArray.size())];
		if (!frameData.m_rtPipeline)
		{
			return;
		}

		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto clusterComputeData = dataTable.GetPassData<ClusterComputeData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();
		uint32_t depthIndex = Engine::GetFrameIndex(depthData->depthTextures.size());
		uint32_t computeIndex = Engine::GetFrameIndex(clusterComputeData->computeMaterials.size());

		BufferDataPtr buffer = clusterComputeData->computeMaterials[computeIndex]->GetStorageBuffer("clusterLightsData");

		// barriers ----------------------------------------------
		BufferMemoryBarrier clusterDataBarrier = buffer->GetBuffer().CreateMemoryBarrier(
			0, 0,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead);
		ImageMemoryBarrier attachmentBarrier = m_visibilityTex->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eGeneral,
			vk::AccessFlagBits::eShaderRead,
			vk::AccessFlagBits::eShaderWrite,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		ImageMemoryBarrier depthTextureBarrier = depthData->depthTextures[depthIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
			0, 1, 0, 1);
		std::vector<ImageMemoryBarrier> barriers{ attachmentBarrier, depthTextureBarrier };
		for (auto tex : m_visibilityTextures)
		{
			ImageMemoryBarrier visibilityBarrier = tex->GetImage().CreateLayoutBarrier(
				ImageLayout::eUndefined,
				ImageLayout::eGeneral,
				vk::AccessFlagBits::eShaderRead,
				vk::AccessFlagBits::eShaderWrite,
				vk::ImageAspectFlagBits::eColor,
				0, 1, 0, 1);
			barriers.push_back(visibilityBarrier);
		}
		vk::ImageMemoryBarrier albedoBarrier = gbufferData->albedos[depthIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		barriers.push_back(albedoBarrier);
		vk::ImageMemoryBarrier normalsBarrier = gbufferData->normals[depthIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		barriers.push_back(normalsBarrier);

		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::PipelineStageFlagBits::eRayTracingShaderKHR,
			vk::DependencyFlags(),
			0, nullptr,
			1, &clusterDataBarrier,
			static_cast<uint32_t>(barriers.size()), barriers.data());

		commandBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, frameData.m_rtPipeline);
		commandBuffer->bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR,
			frameData.m_rtPipelineLayout, 0,
			static_cast<uint32_t>(frameData.m_nativeSets.size()),
			frameData.m_nativeSets.data(),
			0, nullptr);

		ShaderBindingTable& globalSBT = rtScene->GetGlobalShaderBindingTable();

		vk::StridedDeviceAddressRegionKHR rayGenRegion;
		rayGenRegion.setDeviceAddress(globalSBT.GetBuffer()->GetDeviceAddress());
		rayGenRegion.setSize(globalSBT.GetRayGenGroupsSizeBytes());
		rayGenRegion.setStride(globalSBT.GetHandleSizeAlignedBytes());

		vk::StridedDeviceAddressRegionKHR rayMissRegion;
		rayMissRegion.setDeviceAddress(globalSBT.GetBuffer()->GetDeviceAddress() + globalSBT.GetMissGroupsOffsetBytes());
		rayMissRegion.setSize(globalSBT.GetMissGroupsSizeBytes());
		rayMissRegion.setStride(globalSBT.GetHandleSizeAlignedBytes());

		vk::StridedDeviceAddressRegionKHR rayHitRegion;
		rayHitRegion.setDeviceAddress(globalSBT.GetBuffer()->GetDeviceAddress() + globalSBT.GetHitGroupsOffsetBytes());
		rayHitRegion.setSize(globalSBT.GetHitGroupsSizeBytes());
		rayHitRegion.setStride(globalSBT.GetHandleSizeAlignedBytes());

		commandBuffer->traceRaysKHR(rayGenRegion, rayMissRegion, rayHitRegion, { 0,0,0 }, executeContext.GetWidth() / 2, executeContext.GetHeight() / 2, 1);
	}

	void RTShadowPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		RtScene* rtScene = Singleton<RtScene>::GetInstance();
		Renderer* renderer = Engine::GetRendererInstance();
		VulkanDevice& device = renderer->GetVulkanDevice();

		m_visibilityTex = ResourceUtils::CreateColorTexture(
			"RtShadowsVisibilityTexture",
			initContext.GetWidth()/* / 2*/,
			initContext.GetHeight()/* / 2*/,
			vk::Format::eR32Uint,//R8G8B8A8Unorm, 
			true);

		std::string name("RtShadowsVisibilityTexture");
		for (uint32_t idx = 0; idx < 16; idx++)
		{
			Texture2DPtr visibilityTex = ResourceUtils::CreateColorTexture(
				name + std::to_string(idx),
				initContext.GetWidth() / 2,
				initContext.GetHeight() / 2,
				vk::Format::eR8G8B8A8Unorm,
				true);
			m_visibilityTextures.push_back(visibilityTex);
		}
		auto data = dataTable.CreatePassData<RTShadowsData>();
		data->visibilityTextures = m_visibilityTextures;
		data->visibilityTex = m_visibilityTex;

		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto clusterData = dataTable.GetPassData<ClusterComputeData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();

		uint32_t depthCount = depthData->depthTextures.size();
		uint32_t gbufferCount = gbufferData->albedos.size();
		uint32_t clusterDataCount = clusterData->computeMaterials.size();
		assert((depthCount == gbufferCount) && (gbufferCount == clusterDataCount));

		m_shaderResourceMappers.resize(depthCount);
		m_frameDataArray.resize(depthCount);
		for (uint32_t idx = 0; idx < depthData->depthTextures.size(); ++idx)
		{
			MaterialPtr clusterMat = clusterData->computeMaterials[idx];

			m_shaderResourceMappers[idx].AddSampledImage("normalTex", gbufferData->normals[idx]);
			m_shaderResourceMappers[idx].AddSampledImage("depthTex", depthData->depthTextures[idx]);
			m_shaderResourceMappers[idx].AddStorageBuffer("clusterLightsData", clusterMat->GetStorageBuffer("clusterLightsData"));
			m_shaderResourceMappers[idx].AddUniformBuffer("lightsList", clusterMat->GetUniformBuffer("lightsList"));
			m_shaderResourceMappers[idx].AddUniformBuffer("lightsIndices", clusterMat->GetUniformBuffer("lightsIndices"));
			// visibility image
			m_shaderResourceMappers[idx].AddStorageImage("visibilityTex", m_visibilityTex);
			m_shaderResourceMappers[idx].AddStorageImageArray("visibilityTextures", m_visibilityTextures);
			m_shaderResourceMappers[idx].AddAccelerationStructure("tlas", rtScene->GetTlas().accelerationStructure);

			m_frameDataArray[idx].m_rtPipeline = nullptr;
			m_frameDataArray[idx].m_rtPipelineLayout = nullptr;
		}

		m_rayGenShader = DataManager::RequestResourceType<RtShader>("content/shaders/RayGenShadows.spv", ERtShaderType::RST_RAY_GEN);
		m_rayMissShader = DataManager::RequestResourceType<RtShader>("content/shaders/RayMissShadows.spv", ERtShaderType::RST_MISS);
	}

	void RTShadowPass::HandlePreUpdate(std::shared_ptr<GlobalPreFrameMessage> msg)
	{
		RtShadowPassFrameData& frameData = m_frameDataArray[Engine::GetFrameIndex(m_frameDataArray.size())];
		if (!frameData.m_rtPipeline)
		{
			UpdateShaderResources();
			UpdatePipeline();
		}
	}

	void RTShadowPass::UpdateShaderResources()
	{
		Renderer* renderer = Engine::GetRendererInstance();
		VulkanDevice& device = renderer->GetVulkanDevice();
		RtScene* rtScene = Singleton<RtScene>::GetInstance();

		uint32_t frameIndex = Engine::GetFrameIndex(m_shaderResourceMappers.size());
		std::vector<RtShaderPtr>& shaders = rtScene->GetGlobalShaderBindingTable().GetShaders();
		m_shaderResourceMappers[frameIndex].SetShaders(shaders);

		m_shaderResourceMappers[frameIndex].Update();
	}

	void RTShadowPass::UpdatePipeline()
	{
		RtShadowPassFrameData& frameData = m_frameDataArray[Engine::GetFrameIndex(m_frameDataArray.size())];
		uint32_t frameIndex = Engine::GetFrameIndex(m_shaderResourceMappers.size());

		RtScene* rtScene = Singleton<RtScene>::GetInstance();
		if (rtScene->GetGlobalShaderBindingTable().GetShaderGroups().size() == 0)
		{
			return;
		}

		VulkanDevice* device = &Engine::GetRendererInstance()->GetVulkanDevice();
		vk::Device& nativeDevice = device->GetDevice();

		nativeDevice.destroyPipeline(frameData.m_rtPipeline);
		nativeDevice.destroyPipelineLayout(frameData.m_rtPipelineLayout);
		frameData.m_rtPipeline = nullptr;
		frameData.m_rtPipelineLayout = nullptr;
		//------------------------------------------------------------------
		// get descriptor sets from shaders
		frameData.m_sets.clear();
		frameData.m_nativeSets.clear();
		frameData.m_sets = m_shaderResourceMappers[frameIndex].GetDescriptorSets();
		frameData.m_nativeSets.resize(frameData.m_sets.size());
		std::vector<vk::DescriptorSetLayout> descLayouts;
		descLayouts.resize(frameData.m_sets.size());
		for (uint32_t idx = 0; idx < descLayouts.size(); idx++)
		{
			descLayouts[idx] = frameData.m_sets[idx].GetLayout();
			frameData.m_nativeSets[idx] = idx == 0 ? Engine::GetRendererInstance()->GetPerFrameData()->GetSet() : frameData.m_sets[idx].GetSet();
		}
		//------------------------------------------------------------------
		// pipeline layout info
		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo.setSetLayoutCount(static_cast<uint32_t>(descLayouts.size()));
		layoutInfo.setPSetLayouts(descLayouts.data());
		frameData.m_rtPipelineLayout = nativeDevice.createPipelineLayout(layoutInfo);
		//------------------------------------------------------------------
		// pipeline create info
		vk::RayTracingPipelineCreateInfoKHR pipelineInfo;
		pipelineInfo.setGroupCount(static_cast<uint32_t>(rtScene->GetGlobalShaderBindingTable().GetShaderGroups().size()));
		pipelineInfo.setPGroups(rtScene->GetGlobalShaderBindingTable().GetShaderGroups().data());
		pipelineInfo.setStageCount(static_cast<uint32_t>(rtScene->GetGlobalShaderBindingTable().GetShaderStages().size()));
		pipelineInfo.setPStages(rtScene->GetGlobalShaderBindingTable().GetShaderStages().data());
		pipelineInfo.setFlags({});
		pipelineInfo.setLayout(frameData.m_rtPipelineLayout);

		auto pipelineResult = nativeDevice.createRayTracingPipelineKHR(nullptr, nullptr, pipelineInfo);
		if (pipelineResult.result != vk::Result::eSuccess)
		{
			return;
		}
		frameData.m_rtPipeline = pipelineResult.value;
		rtScene->GetGlobalShaderBindingTable().ConstructBuffer(frameData.m_rtPipeline, "rt_scene_sbt_buffer");
	}

}

