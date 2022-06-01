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
		for (auto& data : m_frameDataArray)
		{
			device.destroyPipeline(data.rtPipeline);
			device.destroyPipelineLayout(data.rtPipelineLayout);
		}
	}

	void RTShadowPass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		RtScene* rtScene = Singleton<RtScene>::GetInstance();

		RtShadowPassFrameData& frameData = m_frameDataArray[Engine::GetFrameIndex(m_frameDataArray.size())];
		if (!frameData.rtPipeline)
		{
			return;
		}

		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto clusterComputeData = dataTable.GetPassData<ClusterComputeData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();
		uint32_t depthIndex = Engine::GetFrameIndex(depthData->depthTextures.size());

		// barriers ----------------------------------------------
		BufferMemoryBarrier clusterDataBarrier = clusterComputeData->clusterLightsData->GetBuffer().CreateMemoryBarrier(
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
			barriers.push_back( tex->GetImage().CreateLayoutBarrierColor(ImageLayout::eUndefined, ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eShaderWrite) );
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

		commandBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, frameData.rtPipeline);
		commandBuffer->bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR,
			frameData.rtPipelineLayout, 0,
			static_cast<uint32_t>(frameData.nativeSets.size()),
			frameData.nativeSets.data(),
			0, nullptr);

		ShaderBindingTable& sbt = frameData.sbt;

		vk::StridedDeviceAddressRegionKHR rayGenRegion = sbt.GetRegion(ERtShaderType::RST_RAY_GEN, HashString::NONE);
		vk::StridedDeviceAddressRegionKHR rayMissRegion = sbt.GetRegion(ERtShaderType::RST_MISS, HashString::NONE);
		vk::StridedDeviceAddressRegionKHR rayHitRegion = sbt.GetRegion(ERtShaderType::RST_ANY_HIT, HashString::NONE);

		commandBuffer->traceRaysKHR(rayGenRegion, rayMissRegion, rayHitRegion, { 0,0,0 }, executeContext.GetWidth() / 2, executeContext.GetHeight() / 2, 1);
	}

	void RTShadowPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		RtScene* rtScene = Singleton<RtScene>::GetInstance();
		Renderer* renderer = Engine::GetRendererInstance();
		VulkanDevice& device = renderer->GetVulkanDevice();

		m_rayGenShader = DataManager::RequestResourceType<RtShader>("content/shaders/RayGenShadows.spv", ERtShaderType::RST_RAY_GEN);
		m_rayMissShader = DataManager::RequestResourceType<RtShader>("content/shaders/RayMissShadows.spv", ERtShaderType::RST_MISS);

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
		uint32_t clusterDataCount = clusterData->lightsList.size();
		assert((depthCount == gbufferCount) && (gbufferCount == clusterDataCount));

		m_shaderResourceMappers.resize(depthCount);
		m_frameDataArray.resize(depthCount);
		for (uint32_t idx = 0; idx < depthData->depthTextures.size(); ++idx)
		{
			m_shaderResourceMappers[idx].AddSampledImage("normalTex", gbufferData->normals[idx]);
			m_shaderResourceMappers[idx].AddSampledImage("depthTex", depthData->depthTextures[idx]);
			m_shaderResourceMappers[idx].AddStorageBuffer("clusterLightsData", clusterData->clusterLightsData);
			m_shaderResourceMappers[idx].AddUniformBuffer("lightsList", clusterData->lightsList[idx]);
			m_shaderResourceMappers[idx].AddUniformBuffer("lightsIndices", clusterData->lightsIndices[idx]);
			// visibility image
			m_shaderResourceMappers[idx].AddStorageImage("visibilityTex", m_visibilityTex);
			m_shaderResourceMappers[idx].AddStorageImageArray("visibilityTextures", m_visibilityTextures);
			m_shaderResourceMappers[idx].AddAccelerationStructure("tlas", rtScene->GetTlas().accelerationStructure);

			m_frameDataArray[idx].rtPipeline = nullptr;
			m_frameDataArray[idx].rtPipelineLayout = nullptr;
			m_frameDataArray[idx].sbt.AddShaders({m_rayGenShader, m_rayMissShader});
			m_frameDataArray[idx].sbt.Update();
		}
	}

	void RTShadowPass::HandlePreUpdate(std::shared_ptr<GlobalPreFrameMessage> msg)
	{
		RtShadowPassFrameData& frameData = m_frameDataArray[Engine::GetFrameIndex(m_frameDataArray.size())];
		if (!frameData.rtPipeline)
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
		m_shaderResourceMappers[frameIndex].SetShaders(std::vector<RtShaderPtr>{m_rayGenShader, m_rayMissShader});
		m_shaderResourceMappers[frameIndex].Update();
	}

	void RTShadowPass::UpdatePipeline()
	{
		RtShadowPassFrameData& frameData = m_frameDataArray[Engine::GetFrameIndex(m_frameDataArray.size())];
		uint32_t frameIndex = Engine::GetFrameIndex(m_shaderResourceMappers.size());

		RtScene* rtScene = Singleton<RtScene>::GetInstance();
		if (frameData.sbt.GetShaderGroups().size() == 0)
		{
			return;
		}

		VulkanDevice* device = &Engine::GetRendererInstance()->GetVulkanDevice();
		vk::Device& nativeDevice = device->GetDevice();

		nativeDevice.destroyPipeline(frameData.rtPipeline);
		nativeDevice.destroyPipelineLayout(frameData.rtPipelineLayout);
		frameData.rtPipeline = nullptr;
		frameData.rtPipelineLayout = nullptr;
		//------------------------------------------------------------------
		// get descriptor sets from shaders
		frameData.sets.clear();
		frameData.nativeSets.clear();
		frameData.sets = m_shaderResourceMappers[frameIndex].GetDescriptorSets();
		frameData.nativeSets.resize(frameData.sets.size());
		std::vector<vk::DescriptorSetLayout> descLayouts;
		descLayouts.resize(frameData.sets.size());
		for (uint32_t idx = 0; idx < descLayouts.size(); idx++)
		{
			descLayouts[idx] = frameData.sets[idx].GetLayout();
			frameData.nativeSets[idx] = idx == 0 ? Engine::GetRendererInstance()->GetPerFrameData()->GetSet() : frameData.sets[idx].GetSet();
		}
		//------------------------------------------------------------------
		// pipeline layout info
		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo.setSetLayoutCount(static_cast<uint32_t>(descLayouts.size()));
		layoutInfo.setPSetLayouts(descLayouts.data());
		frameData.rtPipelineLayout = nativeDevice.createPipelineLayout(layoutInfo);
		//------------------------------------------------------------------
		// pipeline create info
		vk::RayTracingPipelineCreateInfoKHR pipelineInfo;
		pipelineInfo.setGroupCount(static_cast<uint32_t>(frameData.sbt.GetShaderGroups().size()));
		pipelineInfo.setPGroups(frameData.sbt.GetShaderGroups().data());
		pipelineInfo.setStageCount(static_cast<uint32_t>(frameData.sbt.GetShaderStages().size()));
		pipelineInfo.setPStages(frameData.sbt.GetShaderStages().data());
		pipelineInfo.setFlags({});
		pipelineInfo.setLayout(frameData.rtPipelineLayout);

		auto pipelineResult = nativeDevice.createRayTracingPipelineKHR(nullptr, nullptr, pipelineInfo);
		if (pipelineResult.result != vk::Result::eSuccess)
		{
			return;
		}
		frameData.rtPipeline = pipelineResult.value;
		frameData.sbt.ConstructBuffer(frameData.rtPipeline, "shadowpass_sbt_buffer");
	}

}

