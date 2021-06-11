#include "RtScene.h"
#include "core/Engine.h"
#include "scene/mesh/MeshComponent.h"
#include "data/DataManager.h"
#include "Renderer.h"

namespace CGE
{

	RtScene::RtScene()
		: m_frameIndexTruncated(0)
	{
		m_blasTable.reserve(1024 * 8);
		// message handlers
		m_messageSubscriber.AddHandler<GlobalUpdateMessage>(this, &RtScene::HandleUpdate);
		m_messageSubscriber.AddHandler<GlobalFlipMessage>(this, &RtScene::HandleFlip);
	}

	RtScene::~RtScene()
	{

	}

	void RtScene::BuildMeshBlases(vk::CommandBuffer* cmdBuff)
	{
		vk::Device& device = Engine::GetRendererInstance()->GetDevice();

		AccelStructuresBuildInfos& accBuildInfos = m_buildInfosArray[m_frameIndexTruncated];

		uint32_t counter = 0;
		std::unordered_map<HashString, ResourcePtr>& meshesTable = DataManager::GetInstance()->GetResourcesTable<MeshData>();
		for (auto& pair : meshesTable)
		{
			MeshDataPtr meshData = ObjectBase::Cast<MeshData>(pair.second);
			HashString resId = pair.first;
			if (m_blasTable.find(resId) != m_blasTable.end())
			{
				continue;
			}

			AccelStructure& as = m_blasTable[resId];

			using BASF = vk::BuildAccelerationStructureFlagBitsKHR;

			{
				vk::AccelerationStructureGeometryTrianglesDataKHR trisData;
				trisData.setVertexFormat(vk::Format::eR32G32B32Sfloat);
				trisData.setVertexStride(sizeof(Vertex));
				trisData.setVertexData(meshData->GetVertexBuffer().GetDeviceAddress());
				trisData.setMaxVertex(meshData->GetVertexCount());
				trisData.setIndexType(vk::IndexType::eUint32);
				trisData.setIndexData(meshData->GetIndexBuffer().GetDeviceAddress());
				trisData.setTransformData({}); // identity

				// it's a union, set triangles or aabb/inst here
				vk::AccelerationStructureGeometryDataKHR geomData;
				geomData.setTriangles(trisData);

				vk::AccelerationStructureGeometryKHR* geometry = new vk::AccelerationStructureGeometryKHR[1];
				geometry[0].setGeometryType(VULKAN_HPP_NAMESPACE::GeometryTypeKHR::eTriangles);
				geometry[0].setGeometry(geomData);
				geometry[0].setFlags(vk::GeometryFlagBitsKHR::eOpaque);
				// store geometries
				accBuildInfos.geometries.push_back(geometry);
			}

			{
				vk::AccelerationStructureBuildGeometryInfoKHR geomInfo;
				geomInfo.setType(VULKAN_HPP_NAMESPACE::AccelerationStructureTypeKHR::eBottomLevel);
				geomInfo.setFlags(BASF::ePreferFastTrace | BASF::eAllowUpdate | BASF::eAllowCompaction);
				geomInfo.setMode(VULKAN_HPP_NAMESPACE::BuildAccelerationStructureModeKHR::eBuild);
				geomInfo.setDstAccelerationStructure(as.accelerationStructure);
				geomInfo.setSrcAccelerationStructure(VK_NULL_HANDLE);
				geomInfo.setGeometryCount(1);
				geomInfo.setPGeometries(accBuildInfos.geometries.back());
				// finally
				accBuildInfos.geometryInfos.push_back(geomInfo);
			}

			uint32_t maxPrimitivesCount = meshData->GetIndexCount() / 3;

			{
				vk::AccelerationStructureBuildRangeInfoKHR* rangeInfo = new vk::AccelerationStructureBuildRangeInfoKHR[1];
				rangeInfo[0].setFirstVertex(0);
				rangeInfo[0].setPrimitiveOffset(0);
				rangeInfo[0].setTransformOffset(0);
				rangeInfo[0].setPrimitiveCount(maxPrimitivesCount);

				accBuildInfos.rangeInfos.push_back(rangeInfo);
			}

			{
				vk::AccelerationStructureBuildSizesInfoKHR sizesInfo;
				device.getAccelerationStructureBuildSizesKHR(
					VULKAN_HPP_NAMESPACE::AccelerationStructureBuildTypeKHR::eDevice,
					&accBuildInfos.geometryInfos.back(),
					&accBuildInfos.rangeInfos.back()->primitiveCount,
					&sizesInfo
				);
				accBuildInfos.buildSizes.push_back(sizesInfo);
			}

			{
				VulkanBuffer scratchBuffer;
				scratchBuffer.createInfo.setSize(accBuildInfos.buildSizes.back().buildScratchSize);
				scratchBuffer.createInfo.setUsage(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress);
				scratchBuffer.createInfo.setSharingMode(VULKAN_HPP_NAMESPACE::SharingMode::eExclusive);
				scratchBuffer.Create(&Engine::GetRendererInstance()->GetVulkanDevice());
				scratchBuffer.BindMemory(vk::MemoryPropertyFlagBits::eDeviceLocal);
				// add scratch buffer
				accBuildInfos.scratchBuffers.push_back(scratchBuffer);
				// set scratch buffer
				accBuildInfos.geometryInfos.back().setScratchData(accBuildInfos.scratchBuffers.back().GetDeviceAddress());
			}

			{
				// setup buffer now that we have a size
				as.buffer.createInfo.setSize(accBuildInfos.buildSizes.back().accelerationStructureSize);
				as.buffer.createInfo.setUsage(vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);
				as.buffer.createInfo.setSharingMode(VULKAN_HPP_NAMESPACE::SharingMode::eExclusive);
				as.buffer.Create(&Engine::GetRendererInstance()->GetVulkanDevice());
				as.buffer.BindMemory(vk::MemoryPropertyFlagBits::eDeviceLocal);
			}

			{
				vk::AccelerationStructureCreateInfoKHR accelStructInfo;
				accelStructInfo.setBuffer(as.buffer);
				accelStructInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel);
				accelStructInfo.setDeviceAddress(as.buffer.GetDeviceAddress());
				accelStructInfo.setCreateFlags({});
				accelStructInfo.setOffset(0);
				accelStructInfo.setSize(accBuildInfos.buildSizes.back().accelerationStructureSize); // accel struct size
				// create acceleration structure
				as.accelerationStructure = device.createAccelerationStructureKHR(accelStructInfo);
			}
		}
		cmdBuff->buildAccelerationStructuresKHR(
			static_cast<uint32_t>( accBuildInfos.geometryInfos.size() ),
			accBuildInfos.geometryInfos.data(),
			accBuildInfos.rangeInfos.data()
		);

		// waiting for blas's to build
		vk::MemoryBarrier barrier;
		barrier.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR);
		barrier.setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureReadKHR);
		cmdBuff->pipelineBarrier(
			vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, 
			vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, 
			vk::DependencyFlags(), 
			1, &barrier, 
			0, nullptr, 0, nullptr);
	}

	void RtScene::BuildSceneTlas(vk::CommandBuffer* cmdBuff)
	{
		Scene* scene = Engine::GetSceneInstance();
		SceneObjectsPack& sceneObjects = scene->GetObjectsPack(false);

		for (SceneObjectComponentPtr sceneComp : sceneObjects.componentsMap[Class::Get<MeshComponent>().GetName()])
		{

		}
	}

	void RtScene::CleanupBuildInfos(AccelStructuresBuildInfos& buildInfos)
	{
		for (uint32_t idx = 0; idx < buildInfos.geometryInfos.size(); idx++)
		{
			delete[] buildInfos.geometries[idx];
			delete[] buildInfos.rangeInfos[idx];
			buildInfos.scratchBuffers[idx].Destroy();
		}

		buildInfos.buildSizes.clear();
		buildInfos.geometries.clear();
		buildInfos.geometryInfos.clear();
		buildInfos.rangeInfos.clear();
		buildInfos.scratchBuffers.clear();
	}

	void RtScene::HandleUpdate(std::shared_ptr<GlobalUpdateMessage> msg)
	{

	}

	void RtScene::HandleFlip(std::shared_ptr<GlobalFlipMessage> msg)
	{
		m_frameIndexTruncated = (m_frameIndexTruncated + 1) % m_buildInfosArray.size();
		CleanupBuildInfos(m_buildInfosArray[m_frameIndexTruncated]);
	}

}

