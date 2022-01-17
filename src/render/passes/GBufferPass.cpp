#include "GBufferPass.h"
#include "utils/ResourceUtils.h"
#include "data/MeshData.h"
#include "scene/mesh/MeshComponent.h"
#include "DepthPrepass.h"

namespace CGE
{
	using namespace VULKAN_HPP_NAMESPACE;

	GBufferPass::GBufferPass(HashString inName)
		:RenderPassBase(inName)
	{
	}
	
	void GBufferPass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{			
		Scene* scene = Engine::GetSceneInstance();
			
		RenderPassBeginInfo passBeginInfo;
		passBeginInfo.setRenderPass(executeContext.GetRenderPass());
		passBeginInfo.setFramebuffer(executeContext.GetFramebuffer());
		passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), Extent2D(executeContext.GetWidth(), executeContext.GetHeight())));
		passBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
		passBeginInfo.setPClearValues(clearValues.data());
			
		DeviceSize offset = 0;
		commandBuffer->beginRenderPass(passBeginInfo, SubpassContents::eInline);
			
		//------------------------------------------------------------------------------------------------------------
		for (HashString& shaderHash : scene->GetShadersList())
		{
			PipelineData& pipelineData = executeContext.FindPipeline(scene->GetShaderToMaterial()[shaderHash][0]);
			
			commandBuffer->bindPipeline(PipelineBindPoint::eGraphics, pipelineData.pipeline);
			commandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});
			
			for (MaterialPtr material : scene->GetShaderToMaterial()[shaderHash])
			{
				HashString materialId = material->GetResourceId();
			
				commandBuffer->bindDescriptorSets(
					PipelineBindPoint::eGraphics,
					pipelineData.pipelineLayout,
					1,
					material->GetDescriptorSets().size()-1,
					material->GetDescriptorSets().data()+1,
					0, nullptr);
			
				for (MeshDataPtr meshData : scene->GetMaterialToMeshData()[material->GetResourceId()])
				{
					HashString meshId = meshData->GetResourceId();
			
					commandBuffer->pushConstants(pipelineData.pipelineLayout, ShaderStageFlagBits::eAll, 0, sizeof(uint32_t), & scene->GetMeshDataToIndex(materialId)[meshId]);
					commandBuffer->bindVertexBuffers(0, 1, &meshData->GetVertexBuffer()->GetNativeBuffer(), &offset);
					commandBuffer->bindIndexBuffer(meshData->GetIndexBuffer()->GetNativeBuffer(), 0, IndexType::eUint32);
					commandBuffer->drawIndexed(meshData->GetIndexCount(), static_cast<uint32_t>(scene->GetMeshDataToTransform(materialId)[meshId].size()), 0, 0, 0);
				}
			}		
		}
		//------------------------------------------------------------------------------------------------------------
		commandBuffer->endRenderPass();
	}

	void GBufferPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		// just init clear values
		clearValues[0].setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));
		clearValues[1].setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));
		clearValues[2].setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));
		clearValues[3].setDepthStencil(ClearDepthStencilValue(1.0f, 0));

		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		initContext.SetDepthAttachments(depthData->depthTextures, false);

		initContext.depthInfo.setDepthTestEnable(VK_TRUE);
		initContext.depthInfo.setDepthCompareOp(vk::CompareOp::eEqual);
		initContext.depthInfo.setDepthWriteEnable(VK_FALSE);
		initContext.depthInfo.setStencilTestEnable(VK_FALSE);

		std::shared_ptr<GBufferPassData> gbufferData = std::make_shared<GBufferPassData>();
		dataTable.AddPassData<GBufferPassData>(gbufferData);

		gbufferData->albedos = ResourceUtils::CreateColorTextureArray(initContext.GetPassName() + HashString("_albedo"), 2, initContext.GetWidth(), initContext.GetHeight());
		gbufferData->normals = ResourceUtils::CreateColorTextureArray(initContext.GetPassName() + HashString("_normal"), 2, initContext.GetWidth(), initContext.GetHeight(), vk::Format::eR16G16B16A16Sfloat);
		gbufferData->velocity = ResourceUtils::CreateColorTextureArray(initContext.GetPassName() + HashString("_velocity"), 2, initContext.GetWidth(), initContext.GetHeight(), vk::Format::eR16G16Sfloat);

		initContext.SetAttachments(0, gbufferData->albedos, true);
		initContext.SetAttachments(1, gbufferData->normals, true);
		initContext.SetAttachments(2, gbufferData->velocity, true);
	}
	
}
