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
#include "import/MeshImporter.h"
#include "DeferredLightingPass.h"
#include "UpdateGIProbesPass.h"


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
		uint32_t prevFrameIndex = Engine::GetPreviousFrameIndex(m_frameData.size());
		RTGIPassFrameData& frameData = m_frameData[frameIndex];
		if (!frameData.pipeline)
		{
			return;
		}

		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto clusterComputeData = dataTable.GetPassData<ClusterComputeData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();
		auto rtShadowData = dataTable.GetPassData<RTShadowsData>();
		auto directLightingData = dataTable.GetPassData<DeferredLightingData>();

		uint32_t depthIndex = Engine::GetFrameIndex(depthData->depthTextures.size());
		uint32_t prevDepthIndex = Engine::GetPreviousFrameIndex(depthData->depthTextures.size());

		// barriers ----------------------------------------------
		BufferMemoryBarrier clusterDataBarrier = clusterComputeData->clusterLightsData->GetBuffer().CreateMemoryBarrier(
			0, 0,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead);
		BufferMemoryBarrier gridDataBarrier = clusterComputeData->gridLightsData->GetBuffer().CreateMemoryBarrier(
			0, 0,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead);
		std::vector<BufferMemoryBarrier> buffersBarriers { clusterDataBarrier, gridDataBarrier };

		std::vector<ImageMemoryBarrier> imageBarriers;
		ImageMemoryBarrier depthTextureBarrier = depthData->depthTextures[depthIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
			0, 1, 0, 1);
		imageBarriers.push_back(depthTextureBarrier);
		ImageMemoryBarrier prevDepthTextureBarrier = depthData->depthTextures[prevDepthIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
			0, 1, 0, 1);
		imageBarriers.push_back(prevDepthTextureBarrier);
		for (auto tex : rtShadowData->visibilityTextures)
		{
			imageBarriers.push_back( tex->GetImage().CreateLayoutBarrierColor(ImageLayout::eGeneral, ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead) );
		}
		vk::ImageMemoryBarrier albedoBarrier = gbufferData->albedos[depthIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		imageBarriers.push_back(albedoBarrier);
		vk::ImageMemoryBarrier normalsBarrier = gbufferData->normals[depthIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		imageBarriers.push_back(normalsBarrier);
		vk::ImageMemoryBarrier velocityBarrier = gbufferData->velocity[depthIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		imageBarriers.push_back(velocityBarrier);
		// direct lighting data
		vk::ImageMemoryBarrier directLightBarrier = directLightingData->hdrRenderTargets[depthIndex]->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		imageBarriers.push_back(directLightBarrier);
		//---------------------------------------------------------------------------------------------------------------------------
		//imageBarriers.push_back(m_lightingData[frameIndex]->GetImage().ToLayout(vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite).CreateCurrentLayoutBarrierColor());

		//---------------------------------------------------------------------------------------------------------------------------
		// ddgi data
		vk::BufferMemoryBarrier probesBufferBarrier = m_probeGridBuffer->GetBuffer().CreateMemoryBarrier(
			0, 0,
			vk::AccessFlagBits::eShaderRead,
			vk::AccessFlagBits::eShaderWrite);
		buffersBarriers.push_back(probesBufferBarrier);

		vk::ImageMemoryBarrier probesImageBarrier = m_probeGridTexture->GetImage().CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eGeneral,
			vk::AccessFlagBits::eShaderRead,
			vk::AccessFlagBits::eShaderWrite,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		imageBarriers.push_back(probesImageBarrier);
		//---------------------------------------------------------------------------------------------------------------------------
		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands | vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eRayTracingShaderKHR,
			vk::PipelineStageFlagBits::eRayTracingShaderKHR,
			vk::DependencyFlags(),
			1, &m_memBar,
			static_cast<uint32_t>(buffersBarriers.size()), buffersBarriers.data(),
			static_cast<uint32_t>(imageBarriers.size()), imageBarriers.data());

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
		vk::StridedDeviceAddressRegionKHR rayGenDDGIRegion = sbt.GetRegion(ERtShaderType::RST_RAY_GEN, m_rayGenDDGI->GetResourceId());
		vk::StridedDeviceAddressRegionKHR rayMissRegion = sbt.GetRegion(ERtShaderType::RST_MISS, m_rayMiss->GetResourceId());
		vk::StridedDeviceAddressRegionKHR rayHitRegion = sbt.GetRegion(ERtShaderType::RST_ANY_HIT, HashString::NONE);

		// screen space probe tracing
		commandBuffer->traceRaysKHR(rayGenRegion, rayMissRegion, rayHitRegion, { 0,0,0 }, executeContext.GetWidth() / 8, executeContext.GetHeight() / 8, 1);
		// dispatch ddgi tracing
		//commandBuffer->traceRaysKHR(rayGenDDGIRegion, rayMissRegion, rayHitRegion, { 0,0,0 }, 32, 16, 32);

		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands | vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eRayTracingShaderKHR,
			vk::DependencyFlags(),
			1, &m_memBar,
			0, nullptr,
			0, nullptr);
	}

	void RTGIPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		m_memBar.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead);
		m_memBar.setDstAccessMask(vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead);

		CreateProbeGridData();

		m_rayGen = DataManager::GetInstance()->RequestResourceByType<RtShader>("content/shaders/RayGenGI.spv", ERtShaderType::RST_RAY_GEN);
		m_rayGenDDGI = DataManager::GetInstance()->RequestResourceByType<RtShader>("content/shaders/RayGenDDGI.spv", ERtShaderType::RST_RAY_GEN);
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
		// initial deferred lighting pass can be used to sample direct lighting in case our rays hit something in screenspace
		auto directLightingData = dataTable.GetPassData<DeferredLightingData>();
		auto giProbesData = dataTable.GetPassData<UpdateGIProbesData>();

		m_temporalCounter = ResourceUtils::CreateColorTexture("RTGI_counter_texture_", initContext.GetWidth() / 8, initContext.GetHeight() / 8, vk::Format::eR32Uint, true);
		m_lightingData = giProbesData->screenProbesData; //ResourceUtils::CreateColorTextureArray("RTGI_light_texture_", 2, initContext.GetWidth(), initContext.GetHeight(), vk::Format::eR16G16B16A16Sfloat, true);//
		m_giDepthData = ResourceUtils::CreateColorTextureArray("RTGI_gi_depth_texture_", 2, initContext.GetWidth() / 8, initContext.GetHeight() / 8, vk::Format::eR32Sfloat, true);

		auto passData = dataTable.CreatePassData<RTGIPassData>();
		passData->lightingData = m_lightingData;
		passData->giDepthData = m_giDepthData;
		passData->probeGridBuffer = m_probeGridBuffer;
		passData->probeGridTexture = m_probeGridTexture;
		passData->probeGridDepthTexture = m_probeGridDepthTexture;

		m_frameData.resize(2);
		for (uint32_t idx = 0; idx < m_frameData.size(); ++idx)
		{
			auto& frameData = m_frameData[idx];
			frameData.pipelineLayout = nullptr;
			frameData.pipeline = nullptr;

			frameData.sbt.AddGlobalRtMaterial(m_globalRTGIMaterial);

			auto& resMapper = frameData.resourceMapper;

			resMapper.AddSampledImage("albedoTex", gbufferData->albedos[idx]);
			resMapper.AddSampledImage("giDepthTex", m_giDepthData[idx]);
			resMapper.AddSampledImage("previousGIDepthTex", m_giDepthData[(idx - 1 + m_giDepthData.size()) % m_giDepthData.size()]);
			resMapper.AddSampledImage("depthTex", depthData->depthTextures[idx]);
			resMapper.AddSampledImage("previousDepthTex", depthData->depthTextures[(idx - 1 + depthData->depthTextures.size()) % depthData->depthTextures.size()]);
			resMapper.AddSampledImage("normalTex", gbufferData->normals[idx]);
			resMapper.AddSampledImage("previousNormalTex", gbufferData->normals[(idx - 1 + gbufferData->normals.size()) % gbufferData->normals.size()]);
			resMapper.AddSampledImage("velocityTex", gbufferData->velocity[idx]);
			resMapper.AddSampledImage("previousLightTex", m_lightingData[(idx + m_lightingData.size() - 1) % m_lightingData.size()]);
			resMapper.AddStorageImage("lightTex", m_lightingData[idx]);
			resMapper.AddStorageImage("counterTex", m_temporalCounter);
			// light clustering data
			//resMapper.AddStorageBuffer("clusterLightsData", clusterData->clusterLightsData);
			resMapper.AddStorageBuffer("gridLightsData", clusterData->gridLightsData);
			resMapper.AddUniformBuffer("lightsList", clusterData->lightsList[idx]);
			resMapper.AddUniformBuffer("lightsIndices", clusterData->lightsIndices[idx]);
			// DDGI grid data
			resMapper.AddStorageBuffer("probesBuffer", m_probeGridBuffer);
			resMapper.AddStorageImage("probesImage", m_probeGridTexture);
			resMapper.AddStorageImage("probesDepthImage", m_probeGridDepthTexture);
			// rt AS data
			resMapper.AddAccelerationStructure("tlas", rtScene->GetTlas().accelerationStructure);
			// rt light visibility data
			resMapper.AddSampledImageArray("visibilityTextures", rtShadowData->visibilityTextures);
			// direct lighting info from deferred lighting pass
			resMapper.AddSampledImage("directLightTex", directLightingData->hdrRenderTargets[idx]);
			// shaders
			resMapper.SetShaders(std::vector<RtShaderPtr>{ m_rayGen, m_rayGenDDGI, m_rayMiss });

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

	void RTGIPass::CreateProbeGridData()
	{
		uint64_t probesTableSize = sizeof(DDGIProbe) * 32 * 32 * 16;
		DDGIProbe probes[32][16][32];
		glm::vec3 beginning = glm::vec3(-31.0f * 0.5f, -15.0f * 0.5f, -31.0f * 0.5f);
		for (uint32_t x = 0; x < 32; ++x)
		{
			for (uint32_t y = 0; y < 16; ++y)
			{
				for (uint32_t z = 0; z < 32; ++z)
				{
					DDGIProbe& probe = probes[x][y][z];

					glm::uint textureCoords = z * 8;
					textureCoords |= ((x * 16 + y) * 8) << 16;
					glm::uint depthCoords = z * 18;
					depthCoords |= ((x * 16 + y) * 18) << 16;

					probe.position = glm::vec4(beginning + glm::vec3(x,y,z), 1.0f);
					probe.texturePosition = textureCoords;
					probe.depthPosition = depthCoords;
					probe.temporalCounter = 0;
				}
			}
		}

		m_probeGridBuffer = ResourceUtils::CreateBufferData("DDGI_grid_buffer", probesTableSize, vk::BufferUsageFlagBits::eStorageBuffer, true);
		m_probeGridBuffer->CopyTo(probesTableSize, reinterpret_cast<const char*>(probes));
		m_probeGridTexture = ResourceUtils::CreateColorTexture("DDGI_grid_texture", 4096, 256, vk::Format::eR16G16B16A16Sfloat, true);
		m_probeGridDepthTexture = ResourceUtils::CreateColorTexture("DDGI_grid_depth_texture", 9216, 576, vk::Format::eR16G16Sfloat, true);
	}

}

