#include "ClusterComputePass.h"
#include "scene/Scene.h"
#include "core/Engine.h"
#include "scene/light/LightComponent.h"
#include "glm/fwd.hpp"
#include "scene/SceneObjectComponent.h"
#include "scene/SceneObjectBase.h"
#include "scene/Transform.h"
#include "../ClusteringManager.h"
#include "utils/Singleton.h"
#include "DepthPrepass.h"
#include "data/DataManager.h"

namespace CGE
{

	ClusterComputePass::ClusterComputePass()
		: RenderPassBase("ClusterComputePass")
	{
		m_subscriber.AddHandler<GlobalPostSceneMessage>(this, &ClusterComputePass::HandleUpdate);
	}

	ClusterComputePass::~ClusterComputePass()
	{

	}

	void ClusterComputePass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		// barriers ----------------------------------------------
		ImageMemoryBarrier depthTextureBarrier = depthTexture->GetImage().CreateLayoutBarrier(
			vk::ImageLayout::eDepthStencilAttachmentOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
			0, 1, 0, 1);
		BufferDataPtr clusterBuffer = computeMaterial->GetStorageBuffer("clusterLightsData");
		BufferMemoryBarrier clusterDataBarrier = clusterBuffer->GetBuffer().CreateMemoryBarrier(
			0, 0,
			vk::AccessFlagBits::eShaderRead,
			vk::AccessFlagBits::eShaderWrite);
		std::vector<ImageMemoryBarrier> barriers{ depthTextureBarrier };
		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllGraphics,
			vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlags(),
			0, nullptr,
			1, &clusterDataBarrier,
			static_cast<uint32_t>(barriers.size()), barriers.data());

		PipelineData& pipelineData = executeContext.FindPipeline(computeMaterial);

		DeviceSize offset = 0;
		commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, pipelineData.pipeline);
		commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});

		glm::uvec2 numClusters = Singleton<ClusteringManager>::GetInstance()->GetNumClusters();
		commandBuffer->dispatch(numClusters.x, numClusters.y, 1);
	}

	void ClusterComputePass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		initContext.compute = true;

		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		
		computeMaterial = DataManager::RequestResourceType<Material>("ClusterComputeMaterial");
		computeMaterial->SetComputeShaderPath("content/shaders/LightClustering.spv");
		computeMaterial->SetStorageBuffer<ClusterLightsData>("clusterLightsData", *clusterLightData);
		computeMaterial->SetUniformBuffer<LightsList>("lightsList", *lightsList);
		computeMaterial->SetUniformBuffer<LightsIndices>("lightsIndices", *lightsIndices);
		computeMaterial->SetTextureArray("depthTextures", depthData->depthTextures);
		computeMaterial->LoadResources();
	}

	void ClusterComputePass::HandleUpdate(const std::shared_ptr<GlobalPostSceneMessage> msg)
	{
		Scene* scene = Engine::GetSceneInstance();
		std::vector<LightComponentPtr> lights = scene->GetSceneComponentsInFrustumCast<LightComponent>();
		std::map<LightType, std::vector<LightInfo>> sortedLights;
		for (LightComponentPtr lightComp : lights)
		{
			LightInfo info;
			info.direction = glm::vec4(lightComp->GetParent()->transform.GetForwardVector(), 0.0);
			info.position = glm::vec4(lightComp->GetParent()->transform.GetLocation(), 1.0);
			info.color = glm::vec4(lightComp->color, 0.0);
			info.rai.x = lightComp->radius;
			info.rai.y = lightComp->spotHalfAngle;
			info.rai.z = lightComp->intensity;

			sortedLights[lightComp->type].push_back(info);
		}
		lightsIndices->directionalPosition.x = 0;
		lightsIndices->directionalPosition.y = static_cast<uint32_t>(sortedLights[LT_Directional].size());
		lightsIndices->pointPosition.x = lightsIndices->directionalPosition.y;
		lightsIndices->pointPosition.y = static_cast<uint32_t>(sortedLights[LT_Point].size());
		lightsIndices->spotPosition.x = lightsIndices->pointPosition.x + lightsIndices->pointPosition.y;
		lightsIndices->spotPosition.y = static_cast<uint32_t>(sortedLights[LT_Spot].size());

		std::memcpy(&lightsList->lights[lightsIndices->directionalPosition.x], sortedLights[LT_Directional].data(), sizeof(LightInfo) * lightsIndices->directionalPosition.y);
		std::memcpy(&lightsList->lights[lightsIndices->spotPosition.x], sortedLights[LT_Spot].data(), sizeof(LightInfo) * lightsIndices->spotPosition.y);
		std::memcpy(&lightsList->lights[lightsIndices->pointPosition.x], sortedLights[LT_Point].data(), sizeof(LightInfo) * lightsIndices->pointPosition.y);

		computeMaterial->UpdateUniformBuffer<LightsList>("lightsList", *lightsList);
		computeMaterial->UpdateUniformBuffer<LightsIndices>("lightsIndices", *lightsIndices);
	}

}



