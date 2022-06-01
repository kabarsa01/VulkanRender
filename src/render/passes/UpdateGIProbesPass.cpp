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
		// screen probes
		imageBarriers.push_back(frameData.screenProbes->GetImage().CreateLayoutBarrierColor(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eShaderWrite));
		//imageBarriers.push_back(frameData.prevScreenProbes->GetImage().CreateLayoutBarrierColor(vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead));

		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands | vk::PipelineStageFlagBits::eRayTracingShaderKHR | vk::PipelineStageFlagBits::eFragmentShader,
			vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlags(),
			0, nullptr,
			0, nullptr,
			static_cast<uint32_t>(imageBarriers.size()), imageBarriers.data());

		{
			PipelineData& pipelineData = executeContext.FindPipeline(Engine::GetForFrame<MaterialPtr>(m_computeMaterials));

			commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, pipelineData.pipeline);
			commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});

			commandBuffer->dispatch(executeContext.GetWidth() / 8, executeContext.GetHeight() / 8, 1);
		}
	}

	void UpdateGIProbesPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		auto gbufferData = dataTable.GetPassData<GBufferPassData>();

		auto probesData = dataTable.CreatePassData<UpdateGIProbesData>();

		probesData->screenProbesData = ResourceUtils::CreateColorTextureArray("RTGI_screen_probes_", 2, initContext.GetWidth(), initContext.GetHeight(), vk::Format::eR16G16B16A16Sfloat, true);

		for (uint32_t idx = 0; idx < 2; idx++)
		{
//			probesData->screenProbesData[idx]->GetImage().ToLayout(vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite);

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
			// finally load
			computeMaterial->LoadResources();

			m_computeMaterials.push_back(computeMaterial);
		}

		initContext.compute = true;
	}


}

