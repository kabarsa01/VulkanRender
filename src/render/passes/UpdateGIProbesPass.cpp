#include "UpdateGIProbesPass.h"
#include "utils/ResourceUtils.h"
#include "data/DataManager.h"
#include "GBufferPass.h"
#include "DepthPrepass.h"
#include "core/Engine.h"

namespace CGE
{


	UpdateGIProbesPass::UpdateGIProbesPass(HashString name)
		: RenderPassBase(name)
	{
	}

	UpdateGIProbesPass::~UpdateGIProbesPass()
	{

	}

	void UpdateGIProbesPass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();
		auto probesData = dataTable.GetPassData<UpdateGIProbesData>();

		FrameData frameData = Engine::GetForFrame<FrameData>(m_frameData);

		std::vector<vk::ImageMemoryBarrier> imageBarriers;
		imageBarriers.push_back(frameData.depth->GetImage().CreateLayoutBarrierDepthStencil(ImageLayout::eUndefined, ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead));
		imageBarriers.push_back(frameData.prevDepth->GetImage().CreateLayoutBarrierDepthStencil(ImageLayout::eUndefined, ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead));
		imageBarriers.push_back(frameData.normal->GetImage().CreateLayoutBarrierColor(ImageLayout::eUndefined, ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead));
		imageBarriers.push_back(frameData.prevNormal->GetImage().CreateLayoutBarrierColor(ImageLayout::eUndefined, ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead));
		imageBarriers.push_back(frameData.velocity->GetImage().CreateLayoutBarrierColor(ImageLayout::eUndefined, ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead));
		// screen probes and irradiance
		imageBarriers.push_back(frameData.screenProbes->GetImage().CreateLayoutBarrierColor(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eShaderWrite));
		imageBarriers.push_back(frameData.irradiance->GetImage().CreateLayoutBarrierColor(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eShaderWrite));
		//imageBarriers.push_back(frameData.prevScreenProbes->GetImage().CreateLayoutBarrierColor(vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead));

		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands | vk::PipelineStageFlagBits::eRayTracingShaderKHR | vk::PipelineStageFlagBits::eFragmentShader,
			vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlags(),
			0, nullptr,
			0, nullptr,
			static_cast<uint32_t>(imageBarriers.size()), imageBarriers.data());

		{
			PipelineData& pipelineData = executeContext.FindPipeline(Engine::GetForFrame<MaterialPtr>(m_giProbesComputeMaterials));

			commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, pipelineData.pipeline);
			commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});

			commandBuffer->dispatch(executeContext.GetWidth() / 8, executeContext.GetHeight() / 8, 1);
		}
		{
			PipelineData& pipelineData = executeContext.FindPipeline(Engine::GetForFrame<MaterialPtr>(m_irradianceComputeMaterials));

			commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, pipelineData.pipeline);
			commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});

			commandBuffer->dispatch(glm::ceil((executeContext.GetWidth() / 4) / 8.0f), glm::ceil((executeContext.GetHeight() / 4) / 8.0), 1);
		}
	}

	void UpdateGIProbesPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();

		auto probesData = dataTable.CreatePassData<UpdateGIProbesData>();

		probesData->screenProbesData = ResourceUtils::CreateColorTextureArray("RTGI_screen_probes_", 2, initContext.GetWidth(), initContext.GetHeight(), vk::Format::eR16G16B16A16Sfloat, true);
		probesData->irradianceData = ResourceUtils::CreateColorTextureArray("RTGI_irradiance_", 2, initContext.GetWidth() / 4, initContext.GetHeight() / 4, vk::Format::eR16G16B16A16Sfloat, true);

		for (uint32_t idx = 0; idx < 2; idx++)
		{
			uint32_t prevIdx = (idx + 1) % 2;

			FrameData frameData;
			// current RTs
			frameData.depth = depthData->depthTextures[idx];
			frameData.normal = gbufferData->normals[idx];
			frameData.velocity = gbufferData->velocity[idx];
			// previous RTs
			frameData.prevDepth = depthData->depthTextures[prevIdx];
			frameData.prevNormal = gbufferData->normals[prevIdx];
			// screen probes
			frameData.screenProbes = probesData->screenProbesData[idx];
			frameData.prevScreenProbes = probesData->screenProbesData[prevIdx];
			// screen irradiance
			frameData.irradiance = probesData->irradianceData[idx];
			frameData.prevIrradiance = probesData->irradianceData[prevIdx];
			// push frame data
			m_frameData.push_back(frameData);

			MaterialPtr computeMaterial = DataManager::RequestResourceType<Material>("UpdateGIProbesComputeMaterial_" + std::to_string(idx));
			computeMaterial->SetComputeShaderPath("content/shaders/UpdateGIProbes.spv");
			// frame data textures
			computeMaterial->SetTexture("depthTexture", frameData.depth);
			computeMaterial->SetTexture("normalTexture", frameData.normal);
			computeMaterial->SetTexture("velocityTexture", frameData.velocity);
			// previous frame textures
			computeMaterial->SetTexture("prevDepthTexture", frameData.prevDepth);
			computeMaterial->SetTexture("prevNormalTexture", frameData.prevNormal);
			// screen probes textures
			computeMaterial->SetStorageTexture("probeTexture", frameData.screenProbes);
			computeMaterial->SetTexture("prevProbeTexture", frameData.prevScreenProbes);
			// irradiance textures
			computeMaterial->SetStorageTexture("irradianceTexture", frameData.irradiance);
			computeMaterial->SetTexture("prevIrradianceTexture", frameData.prevIrradiance);
			// finally load
			computeMaterial->LoadResources();

			m_giProbesComputeMaterials.push_back(computeMaterial);

			//--------------------------------------------------------------------------------------------------------------------------------

			MaterialPtr irradianceComputeMaterial = DataManager::RequestResourceType<Material>("UpdateIrradianceComputeMaterial_" + std::to_string(idx));
			irradianceComputeMaterial->SetComputeShaderPath("content/shaders/UpdateIrradiance.spv");
			// frame data textures
			irradianceComputeMaterial->SetTexture("depthTexture", frameData.depth);
			irradianceComputeMaterial->SetTexture("normalTexture", frameData.normal);
			irradianceComputeMaterial->SetTexture("velocityTexture", frameData.velocity);
			// previous frame textures
			irradianceComputeMaterial->SetTexture("prevDepthTexture", frameData.prevDepth);
			irradianceComputeMaterial->SetTexture("prevNormalTexture", frameData.prevNormal);
			// screen probes textures
			irradianceComputeMaterial->SetStorageTexture("probeTexture", frameData.screenProbes);
			irradianceComputeMaterial->SetTexture("prevProbeTexture", frameData.prevScreenProbes);
			// irradiance textures
			irradianceComputeMaterial->SetStorageTexture("irradianceTexture", frameData.irradiance);
			irradianceComputeMaterial->SetTexture("prevIrradianceTexture", frameData.prevIrradiance);
			// finally load
			irradianceComputeMaterial->LoadResources();

			m_irradianceComputeMaterials.push_back(irradianceComputeMaterial);
		}

		initContext.compute = true;
	}


}

