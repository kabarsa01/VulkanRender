#include "LightPropagationComputePass.h"
#include "RTGIPass.h"
#include "data/DataManager.h"

namespace CGE
{

	LightPropagationComputePass::LightPropagationComputePass(HashString name)
		: RenderPassBase(name)
	{

	}

	LightPropagationComputePass::~LightPropagationComputePass()
	{

	}

	void LightPropagationComputePass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		auto giData = dataTable.GetPassData<RTGIPassData>();
		// barriers ----------------------------------------------
		ImageMemoryBarrier gridTextureBarrier = giData->probeGridTexture->GetImage().CreateLayoutBarrier(
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eGeneral,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		ImageMemoryBarrier gridDepthBarrier = giData->probeGridDepthTexture->GetImage().CreateLayoutBarrier(
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1);

		BufferMemoryBarrier probeGridBarrier = giData->probeGridBuffer->GetBuffer().CreateMemoryBarrier(
			0, 0,
			vk::AccessFlagBits::eShaderRead,
			vk::AccessFlagBits::eShaderWrite);

		std::vector<ImageMemoryBarrier> imagesBarriers{ gridTextureBarrier, gridDepthBarrier };
		std::vector<BufferMemoryBarrier> buffersBarriers{ probeGridBarrier };

		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlags(),
			0, nullptr,
			static_cast<uint32_t>(buffersBarriers.size()), buffersBarriers.data(),
			static_cast<uint32_t>(imagesBarriers.size()), imagesBarriers.data());

		{
			PipelineData& gridPipelineData = executeContext.FindPipeline(m_computeMaterial);

			DeviceSize offset = 0;
			commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, gridPipelineData.pipeline);
			commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, gridPipelineData.pipelineLayout, 0, gridPipelineData.descriptorSets, {});

			commandBuffer->dispatch(32, 16, 1);
		}

	}

	void LightPropagationComputePass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		initContext.compute = true;

		auto giData = dataTable.GetPassData<RTGIPassData>();

		m_computeMaterial = DataManager::GetInstance()->RequestResourceByType<Material>(HashString("light_Propagation_material"));
		m_computeMaterial->SetComputeShaderPath("content/shaders/LightPropagation.spv");
		m_computeMaterial->SetStorageBufferExternal("probeGridData", giData->probeGridBuffer);
		m_computeMaterial->SetStorageTexture("probeTexture", giData->probeGridTexture);
		m_computeMaterial->SetTexture("probeDepthTexture", giData->probeGridDepthTexture);
		m_computeMaterial->LoadResources();
	}

}

