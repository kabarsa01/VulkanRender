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
#include "utils/ResourceUtils.h"

namespace CGE
{

	ClusterComputePass::ClusterComputePass(const HashString& name)
		: RenderPassBase(name)
	{
		m_subscriber.AddHandler<GlobalPostSceneMessage>(this, &ClusterComputePass::HandleUpdate);
	}

	ClusterComputePass::~ClusterComputePass()
	{
		delete m_lightsList;
		delete m_lightsIndices;
	}

	void ClusterComputePass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		auto depthData = dataTable.GetPassData<DepthPrepassData>();

		uint32_t materialIndex = Engine::GetFrameIndex(m_computeMaterials.size());
		uint32_t depthIndex = Engine::GetFrameIndex(depthData->depthTextures.size());

		// barriers ----------------------------------------------
		ImageMemoryBarrier depthTextureBarrier = depthData->depthTextures[depthIndex]->GetImage().CreateLayoutBarrier(
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
			0, 1, 0, 1);
		BufferDataPtr clusterBuffer = m_computeMaterials[materialIndex]->GetStorageBuffer("clusterLightsData");
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

		{
			PipelineData& pipelineData = executeContext.FindPipeline(m_computeMaterials[materialIndex]);

			DeviceSize offset = 0;
			commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, pipelineData.pipeline);
			commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});

			glm::uvec2 numClusters = Singleton<ClusteringManager>::GetInstance()->GetNumClusters();
			commandBuffer->dispatch(numClusters.x, numClusters.y, 1);
		}

		{
			PipelineData& gridPipelineData = executeContext.FindPipeline(m_gridComputeMaterials[materialIndex]);

			DeviceSize offset = 0;
			commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, gridPipelineData.pipeline);
			commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, gridPipelineData.pipelineLayout, 0, gridPipelineData.descriptorSets, {});

			commandBuffer->dispatch(25, 25, 6);
		}
	}

	void ClusterComputePass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		m_lightsList = new LightsList();
		m_lightsIndices = new LightsIndices();

		auto clusterDataPtr = dataTable.CreatePassData<ClusterComputeData>();

		initContext.compute = true;
		auto depthData = dataTable.GetPassData<DepthPrepassData>();

		// storing for easier access
		clusterDataPtr->lightsList = ResourceUtils::CreateBufferDataArray("lightsList", 2, sizeof(LightsList), vk::BufferUsageFlagBits::eUniformBuffer, true);
		clusterDataPtr->lightsIndices = ResourceUtils::CreateBufferDataArray("lightsIndices", 2, sizeof(LightsIndices), vk::BufferUsageFlagBits::eUniformBuffer, true);
		// clusters data
		clusterDataPtr->clusterLightsData = ResourceUtils::CreateBufferData("clusterLightsData", sizeof(ClusterLightsData), vk::BufferUsageFlagBits::eStorageBuffer, true);
		// grid data
		clusterDataPtr->gridLightsData = CreateLightsGrid();
		
		for (uint32_t idx = 0; idx < depthData->depthTextures.size(); ++idx)
		{
			MaterialPtr computeMaterial = DataManager::RequestResourceType<Material>("ClusterComputeMaterial_" + std::to_string(idx));
			computeMaterial->SetComputeShaderPath("content/shaders/LightClustering.spv");
			computeMaterial->SetStorageBufferExternal("clusterLightsData", clusterDataPtr->clusterLightsData);
			computeMaterial->SetUniformBufferExternal("lightsList", clusterDataPtr->lightsList[0]);
			computeMaterial->SetUniformBufferExternal("lightsIndices", clusterDataPtr->lightsIndices[0]);
			computeMaterial->SetTexture("depthTexture", depthData->depthTextures[idx]);
			computeMaterial->LoadResources();

			m_computeMaterials.push_back(computeMaterial);

			MaterialPtr gridComputeMaterial = DataManager::RequestResourceType<Material>("GridComputeMaterial_" + std::to_string(idx));
			gridComputeMaterial->SetComputeShaderPath("content/shaders/LightGrid.spv");
			gridComputeMaterial->SetStorageBufferExternal("gridLightsData", clusterDataPtr->gridLightsData);
			gridComputeMaterial->SetUniformBufferExternal("lightsList", clusterDataPtr->lightsList[0]);
			gridComputeMaterial->SetUniformBufferExternal("lightsIndices", clusterDataPtr->lightsIndices[0]);
			gridComputeMaterial->SetTexture("depthTexture", depthData->depthTextures[idx]);
			gridComputeMaterial->LoadResources();

			m_gridComputeMaterials.push_back(gridComputeMaterial);
		}
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
		m_lightsIndices->directionalPosition.x = 0;
		m_lightsIndices->directionalPosition.y = static_cast<uint32_t>(sortedLights[LT_Directional].size());
		m_lightsIndices->pointPosition.x = m_lightsIndices->directionalPosition.y;
		m_lightsIndices->pointPosition.y = static_cast<uint32_t>(sortedLights[LT_Point].size());
		m_lightsIndices->spotPosition.x = m_lightsIndices->pointPosition.x + m_lightsIndices->pointPosition.y;
		m_lightsIndices->spotPosition.y = static_cast<uint32_t>(sortedLights[LT_Spot].size());

		std::memcpy(&m_lightsList->lights[m_lightsIndices->directionalPosition.x], sortedLights[LT_Directional].data(), sizeof(LightInfo) * m_lightsIndices->directionalPosition.y);
		std::memcpy(&m_lightsList->lights[m_lightsIndices->spotPosition.x], sortedLights[LT_Spot].data(), sizeof(LightInfo) * m_lightsIndices->spotPosition.y);
		std::memcpy(&m_lightsList->lights[m_lightsIndices->pointPosition.x], sortedLights[LT_Point].data(), sizeof(LightInfo) * m_lightsIndices->pointPosition.y);

		uint32_t materialIndex = Engine::GetFrameIndex(m_computeMaterials.size());
		m_computeMaterials[materialIndex]->UpdateUniformBuffer<LightsList>("lightsList", *m_lightsList);
		m_computeMaterials[materialIndex]->UpdateUniformBuffer<LightsIndices>("lightsIndices", *m_lightsIndices);

	}

	BufferDataPtr ClusterComputePass::CreateLightsGrid()
	{
		uint64_t headerSizeBytes = sizeof(GridHierarchy);

		GridHierarchy grid;
		grid.gridSpecs = glm::uvec4(25, 25, 25, 6);

		grid.lightsPerCell = 256;

		uint64_t cellSizeBytes = grid.lightsPerCell * 2; // 2 bytes per light index
		uint64_t gridSizeBytes = cellSizeBytes * grid.gridSpecs.x * grid.gridSpecs.y * grid.gridSpecs.z;
		uint64_t gridsSizeBytes = gridSizeBytes * grid.gridSpecs.w;
		uint64_t gridsListSizeBytes = grid.gridSpecs.w * sizeof(vk::DeviceAddress);

		uint64_t totalSize = headerSizeBytes + gridsListSizeBytes + gridsSizeBytes;
		BufferDataPtr buffer = ResourceUtils::CreateBufferData("lights_grid", totalSize, vk::BufferUsageFlagBits::eStorageBuffer, true);

		grid.grids = buffer->GetDeviceAddress() + headerSizeBytes;
		uint64_t gridsOffsetBytes = headerSizeBytes + gridsListSizeBytes;
		char* data = new char[gridsOffsetBytes]();
		memcpy(data, &grid, headerSizeBytes);

		for (uint32_t idx = 0; idx < grid.gridSpecs.w; ++idx)
		{
			vk::DeviceAddress* deviceAddress = reinterpret_cast<vk::DeviceAddress*>(data + headerSizeBytes);
			deviceAddress[idx] = buffer->GetDeviceAddress() + gridsOffsetBytes + (idx * gridSizeBytes);
		}

		buffer->CopyTo(gridsOffsetBytes, data);
		delete[] data;

		return buffer;
	}

}



