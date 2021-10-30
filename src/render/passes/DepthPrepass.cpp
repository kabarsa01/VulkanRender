#include "DepthPrepass.h"
#include "core/Engine.h"
#include "../Renderer.h"
#include "../objects/VulkanDevice.h"
#include "utils/ResourceUtils.h"

namespace CGE
{

	void DepthPrepass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		vk::ClearValue clearValue;
		clearValue.setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));

		Scene* scene = Engine::GetSceneInstance();

		vk::RenderPassBeginInfo passBeginInfo;
		passBeginInfo.setRenderPass(executeContext.GetRenderPass());
		passBeginInfo.setFramebuffer(executeContext.GetFramebuffer());
		passBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(executeContext.GetWidth(), executeContext.GetHeight())));
		passBeginInfo.setClearValueCount(1);
		passBeginInfo.setPClearValues(&clearValue);

		DeviceSize offset = 0;
		commandBuffer->beginRenderPass(passBeginInfo, vk::SubpassContents::eInline);

		//------------------------------------------------------------------------------------------------------------
		for (HashString& shaderHash : scene->GetShadersList())
		{
			PipelineData& pipelineData = executeContext.FindPipeline(scene->GetShaderToMaterial()[shaderHash][0]);

			commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineData.pipeline);
			commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});

			for (MaterialPtr material : scene->GetShaderToMaterial()[shaderHash])
			{
				HashString materialId = material->GetResourceId();

				commandBuffer->bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					pipelineData.pipelineLayout,
					1,
					material->GetDescriptorSets().size() - 1,
					material->GetDescriptorSets().data() + 1,
					0, nullptr);

				for (MeshDataPtr meshData : scene->GetMaterialToMeshData()[material->GetResourceId()])
				{
					HashString meshId = meshData->GetResourceId();

					commandBuffer->pushConstants(pipelineData.pipelineLayout, vk::ShaderStageFlagBits::eAll, 0, sizeof(uint32_t), &scene->GetMeshDataToIndex(materialId)[meshId]);
					commandBuffer->bindVertexBuffers(0, 1, &meshData->GetVertexBuffer().GetBuffer(), &offset);
					commandBuffer->bindIndexBuffer(meshData->GetIndexBuffer().GetBuffer(), 0, vk::IndexType::eUint32);
					commandBuffer->drawIndexed(meshData->GetIndexCount(), static_cast<uint32_t>(scene->GetMeshDataToTransform(materialId)[meshId].size()), 0, 0, 0);
				}
			}
		}
		//------------------------------------------------------------------------------------------------------------
		commandBuffer->endRenderPass();
	}

	void DepthPrepass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		VulkanDevice& device = Engine::GetRendererInstance()->GetVulkanDevice();

		auto passData = std::make_shared<DepthPrepassData>();
		dataTable.AddPassData<DepthPrepassData>(passData);

		passData->depthTextures = ResourceUtils::CreateDepthAttachmentTextures("main_depth", 2, &device, initContext.GetWidth(), initContext.GetHeight());

		initContext.SetDepthAttachments(passData->depthTextures);
	}

}



