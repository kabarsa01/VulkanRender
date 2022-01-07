#include "RTGIPass.h"
#include "utils/ResourceUtils.h"
#include "DepthPrepass.h"
#include "GBufferPass.h"
#include "../RtScene.h"
#include "utils/Singleton.h"
#include "ClusterComputePass.h"
#include "RTShadowPass.h"
#include "data/DataManager.h"
#include "../shader/RtShader.h"
#include "core/Engine.h"
#include "../Renderer.h"
#include "../PerFrameData.h"


namespace CGE
{

	RTGIPass::RTGIPass(HashString name)
		: RenderPassBase(name)
	{
//		m_subscriber.AddHandler<GlobalPostSceneMessage>(this, &RTGIPass::HandleUpdate);
	}

	RTGIPass::~RTGIPass()
	{
		vk::Device& device = Engine::GetRendererInstance()->GetDevice();
		for (auto& data : m_frameData)
		{
			device.destroyPipeline(data.pipeline);
			device.destroyPipelineLayout(data.pipelineLayout);
		}
	}

	void RTGIPass::Update()
	{
		HandleUpdate(nullptr);
	}

	void RTGIPass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		RtScene* rtScene = Singleton<RtScene>::GetInstance();

		uint32_t frameIndex = Engine::GetFrameIndex(m_frameData.size());
		RTGIPassFrameData& frameData = m_frameData[frameIndex];
		if (!frameData.pipeline)
		{
			return;
		}

		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto clusterComputeData = dataTable.GetPassData<ClusterComputeData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();
		auto rtShadowData = dataTable.GetPassData<RTShadowsData>();

		uint32_t depthIndex = Engine::GetFrameIndex(depthData->depthTextures.size());
		uint32_t computeIndex = Engine::GetFrameIndex(clusterComputeData->computeMaterials.size());

		// barriers ----------------------------------------------
		BufferDataPtr buffer = clusterComputeData->clusterLightsData[frameIndex];
		BufferMemoryBarrier clusterDataBarrier = buffer->GetBuffer().CreateMemoryBarrier(
			0, 0,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead);
		ImageMemoryBarrier depthTextureBarrier = depthData->depthTextures[depthIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
			0, 1, 0, 1);
		std::vector<ImageMemoryBarrier> barriers{ depthTextureBarrier };
		for (auto tex : rtShadowData->visibilityTextures)
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

		auto nativeSets = frameData.resourceMapper.GetNativeDescriptorSets();
		nativeSets[0] = Engine::GetRendererInstance()->GetPerFrameData()->GetSet();

		commandBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, frameData.pipeline);
		commandBuffer->bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR,
			frameData.pipelineLayout, 0,
			static_cast<uint32_t>(nativeSets.size()),
			nativeSets.data(),
			0, nullptr);

		ShaderBindingTable& sbt = frameData.sbt;

		vk::StridedDeviceAddressRegionKHR rayGenRegion = sbt.GetRegion(ERtShaderType::RST_RAY_GEN, m_rayGen->GetResourceId());
		vk::StridedDeviceAddressRegionKHR rayMissRegion = sbt.GetRegion(ERtShaderType::RST_MISS, m_rayMiss->GetResourceId());
		vk::StridedDeviceAddressRegionKHR rayHitRegion = sbt.GetRegion(ERtShaderType::RST_ANY_HIT, HashString::NONE);

		commandBuffer->traceRaysKHR(rayGenRegion, rayMissRegion, rayHitRegion, { 0,0,0 }, executeContext.GetWidth() / 2, executeContext.GetHeight() / 2, 1);
	}

	void RTGIPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		m_rayGen = DataManager::GetInstance()->RequestResourceByType<RtShader>("content/shaders/RayGenGI.spv", ERtShaderType::RST_RAY_GEN);
		m_rayMiss = DataManager::GetInstance()->RequestResourceByType<RtShader>("content/shaders/RayMissGI.spv", ERtShaderType::RST_MISS);
		//m_closestHit = DataManager::GetInstance()->RequestResourceByType<RtShader>("content/shaders/RayClosestHitGI.spv", ERtShaderType::RST_CLOSEST_HIT);

		m_globalRTGIMaterial = DataManager::GetInstance()->RequestResourceByType<RtMaterial>("GlobalRTGIMaterial");
		m_globalRTGIMaterial->SetShader(ERtShaderType::RST_CLOSEST_HIT, "content/shaders/RayClosestHitGI.spv", "main");
		m_globalRTGIMaterial->LoadResources();

		RtScene* rtScene = Singleton<RtScene>::GetInstance();

		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();
		auto clusterData = dataTable.GetPassData<ClusterComputeData>();
		auto rtShadowData = dataTable.GetPassData<RTShadowsData>();

		m_lightingData = ResourceUtils::CreateColorTextureArray("RTGI_light_texture_", 2, initContext.GetWidth(), initContext.GetHeight(), vk::Format::eR16G16B16A16Sfloat, true);
		m_frameData.resize(2);

		for (uint32_t idx = 0; idx < m_frameData.size(); ++idx)
		{
			auto& frameData = m_frameData[idx];
			frameData.pipelineLayout = nullptr;
			frameData.pipeline = nullptr;

			auto& resMapper = frameData.resourceMapper;

			resMapper.AddSampledImage("depthTex", depthData->depthTextures[idx]);
			resMapper.AddSampledImage("normalTex", gbufferData->normals[idx]);
			resMapper.AddStorageImage("lightTex", m_lightingData[idx]);
			// light clustering data
			resMapper.AddStorageBuffer("clusterLightsData", clusterData->clusterLightsData[idx]);
			resMapper.AddUniformBuffer("lightsList", clusterData->lightsList[idx]);
			resMapper.AddUniformBuffer("lightsIndices", clusterData->lightsIndices[idx]);
			// rt as data
			resMapper.AddAccelerationStructure("tlas", rtScene->GetTlas().accelerationStructure);
			// rt light visibility data
			resMapper.AddSampledImageArray("visibilityTextures", rtShadowData->visibilityTextures);
			// shaders
			resMapper.SetShaders(std::vector<RtShaderPtr>{ m_rayGen, m_rayMiss });

			resMapper.Update();
		}
	}

	void RTGIPass::HandleUpdate(std::shared_ptr<GlobalPostSceneMessage> msg)
	{
		RtScene* rtScene = Singleton<RtScene>::GetInstance();
		vk::Device nativeDevice = Engine::GetRendererInstance()->GetDevice();

		std::vector<RtShaderPtr> rtShaders = DataManager::GetInstance()->GetResourcesByType<RtShader>();
		std::vector<RtMaterialPtr> rtMaterials = DataManager::GetInstance()->GetResourcesByType<RtMaterial>();

		uint32_t frameIndex = Engine::GetFrameIndex(m_frameData.size());
		auto& frameData = m_frameData[frameIndex];

		if (frameData.pipeline)
		{
			return;
		}

		frameData.sbt.SetShaders(rtShaders);
		frameData.sbt.SetRtMaterials(rtMaterials);
		frameData.sbt.Update();

		nativeDevice.destroyPipeline(frameData.pipeline);
		nativeDevice.destroyPipelineLayout(frameData.pipelineLayout);
		frameData.pipeline = nullptr;
		frameData.pipelineLayout = nullptr;
		//------------------------------------------------------------------
		// pipeline layout
		auto descLayouts = frameData.resourceMapper.GetNativeLayouts();
		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo.setSetLayoutCount(static_cast<uint32_t>(descLayouts.size()));
		layoutInfo.setPSetLayouts(descLayouts.data());
		frameData.pipelineLayout = nativeDevice.createPipelineLayout(layoutInfo);
		//------------------------------------------------------------------
		// pipeline
		frameData.pipeline = RTUtils::CreatePipeline(frameData.pipelineLayout, frameData.sbt);
		if (frameData.pipeline)
		{
			frameData.sbt.ConstructBuffer(frameData.pipeline, "gi_sbt_buffer_");
		}
	}

}

