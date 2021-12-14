#include "RtScene.h"
#include "core/Engine.h"
#include "scene/mesh/MeshComponent.h"
#include "data/DataManager.h"
#include "Renderer.h"
#include "utils/RTUtils.h"
#include "core/ObjectBase.h"
#include "data/RtMaterial.h"
#include "scene/SceneObjectComponent.h"
#include "scene/SceneObjectBase.h"
#include "scene/Transform.h"
#include "utils/ResourceUtils.h"

namespace CGE
{

	constexpr static uint64_t TLAS_SIZE_BYTES = 1024 * 1024 * 8;
	constexpr static uint64_t TLAS_SCRATCH_SIZE_BYTES = 1024 * 1024 * 16;

	RtScene::RtScene()
		: m_frameIndexTruncated(0)
	{
		m_shaderBindingTables.resize(2);
		m_blasTable.reserve(1024 * 8);
		// message handlers
		m_messageSubscriber.AddHandler<GlobalUpdateMessage>(this, &RtScene::HandleUpdate);
		m_messageSubscriber.AddHandler<GlobalPostFrameMessage>(this, &RtScene::HandleFlip);
	}

	RtScene::~RtScene()
	{

	}

	void RtScene::Init()
	{
		// make a buffer for 16K instances
		vk::BufferCreateInfo instanceBuffCreateInfo;
		instanceBuffCreateInfo.setUsage(vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eTransferDst);
		instanceBuffCreateInfo.setSize(sizeof(vk::AccelerationStructureInstanceKHR) * 1024 * 16);
		instanceBuffCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
		m_instancesBuffer = ObjectBase::NewObject<BufferData>("TLAS_instances_buffer", instanceBuffCreateInfo, true);
		m_instancesBuffer->Create();


		// just an experiment, create very big acceleration structure and buffer and just rebuild it in place
		m_tlas.buffer = ResourceUtils::CreateBufferData(
			"RtScene_TLAS",
			TLAS_SIZE_BYTES,
			vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			true
		);
		m_tlasBuildInfo.scratchBuffer = ResourceUtils::CreateBufferData(
			"RtScene_TLAS_scratch",
			TLAS_SCRATCH_SIZE_BYTES,
			vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			true
		);

		vk::AccelerationStructureCreateInfoKHR accelInfo;
		accelInfo.setBuffer(m_tlas.buffer->GetNativeBuffer());
		accelInfo.setOffset(0);
		accelInfo.setSize(TLAS_SIZE_BYTES);
		accelInfo.setType(vk::AccelerationStructureTypeKHR::eTopLevel);
		m_tlas.accelerationStructure = Engine::GetRendererInstance()->GetDevice().createAccelerationStructureKHR(accelInfo);
	}

	void RtScene::Cleanup()
	{
		vk::Device& device = Engine::GetRendererInstance()->GetDevice();

		for (auto& pair : m_blasTable)
		{
			RTUtils::CleanupAccelerationStructure(pair.second);
		}
		RTUtils::CleanupAccelerationStructure(m_tlas);
		RTUtils::CleanupBuildInfo(m_tlasBuildInfo);
		m_instancesBuffer = nullptr;
	}

	void RtScene::UpdateShaders()
	{
		auto& sbt = m_shaderBindingTables[Engine::GetFrameIndex(m_shaderBindingTables.size())];
		sbt.Clear();
		sbt.AddShaders(DataManager::GetInstance()->GetResourcesByType<RtShader>());
		sbt.AddRtMaterials(DataManager::GetInstance()->GetResourcesByType<RtMaterial>());
		sbt.Update();
	}

	void RtScene::UpdateInstances()
	{
		Scene* scene = Engine::GetSceneInstance();
		SceneObjectsPack& sceneObjects = scene->GetObjectsPack(false);
		auto& sbt = m_shaderBindingTables[Engine::GetFrameIndex(m_shaderBindingTables.size())];

		m_instances.clear();

		uint32_t instanceIndex = 0;
		for (SceneObjectComponentPtr sceneComp : sceneObjects.componentsMap[Class::Get<MeshComponent>().GetName()])
		{
			MeshComponentPtr meshComp = ObjectBase::Cast<MeshComponent>(sceneComp);
			if (!meshComp->rtMaterial)
			{
				continue;
			}

			vk::AccelerationStructureInstanceKHR instance = RTUtils::GetAccelerationStructureInstance(meshComp);
			vk::Device& device = Engine::GetRendererInstance()->GetDevice();
			vk::DeviceAddress accelAddr = device.getAccelerationStructureAddressKHR(m_blasTable[meshComp->meshData->GetResourceId()].accelerationStructure);
			instance.setAccelerationStructureReference(accelAddr);
			instance.setFlags(vk::GeometryInstanceFlagBitsKHR::eForceOpaque);
			instance.setInstanceCustomIndex(instanceIndex++);
			uint32_t sbtOffset = sbt.GetHitGroupOffset(meshComp->rtMaterial);
			instance.setInstanceShaderBindingTableRecordOffset(sbtOffset);
			instance.setMask(0xffffffff);

			glm::mat4x4 mat = meshComp->GetParent()->transform.GetMatrix();
			mat = glm::transpose(mat);
			memcpy(&instance.transform, &mat, sizeof(instance.transform));

			m_instances.push_back(instance);
		}

		if (m_instances.size() > 0)
		{
			m_instancesBuffer->CopyTo(
				sizeof(vk::AccelerationStructureInstanceKHR) * m_instances.size(),
				reinterpret_cast<const char*>(m_instances.data()));
			// TODO barrier
		}
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
				vk::AccelerationStructureGeometryTrianglesDataKHR trisData = RTUtils::GetGeometryTrianglesData(meshData);
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
					vk::AccelerationStructureBuildTypeKHR::eDevice,
					&accBuildInfos.geometryInfos.back(),
					&accBuildInfos.rangeInfos.back()->primitiveCount,
					&sizesInfo
				);
				accBuildInfos.buildSizes.push_back(sizesInfo);
			}

			{
				vk::BufferCreateInfo createInfo;
				createInfo.setSize(accBuildInfos.buildSizes.back().buildScratchSize);
				createInfo.setUsage(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress);
				createInfo.setSharingMode(vk::SharingMode::eExclusive);
				BufferDataPtr scratchBuffer = ObjectBase::NewObject<BufferData>(meshData->GetResourceId() + HashString("_BLAS_scratch"), createInfo, true);
				scratchBuffer->Create();
				// add scratch buffer
				accBuildInfos.scratchBuffers.push_back(scratchBuffer);
			}

			{
				// setup buffer now that we have a size
				vk::BufferCreateInfo createInfo;
				createInfo.setSize(accBuildInfos.buildSizes.back().accelerationStructureSize);
				createInfo.setUsage(vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);
				createInfo.setSharingMode(VULKAN_HPP_NAMESPACE::SharingMode::eExclusive);

				as.buffer = ObjectBase::NewObject<BufferData>(meshData->GetResourceId() + HashString("_BLAS"), createInfo, true);
				as.buffer->Create();
			}

			{
				vk::AccelerationStructureCreateInfoKHR accelStructInfo;
				accelStructInfo.setBuffer(as.buffer->GetNativeBuffer());
				accelStructInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel);
				accelStructInfo.setCreateFlags({});
				accelStructInfo.setOffset(0);
				accelStructInfo.setSize(accBuildInfos.buildSizes.back().accelerationStructureSize); // accel struct size
				// create acceleration structure
				as.accelerationStructure = device.createAccelerationStructureKHR(accelStructInfo);
			}

			// update geom info
			accBuildInfos.geometryInfos.back().setScratchData(accBuildInfos.scratchBuffers.back()->GetBuffer().GetDeviceAddress());
			accBuildInfos.geometryInfos.back().setDstAccelerationStructure(as.accelerationStructure);
		}

		if (accBuildInfos.geometryInfos.size() == 0)
		{
			return;
		}

		cmdBuff->buildAccelerationStructuresKHR(
			static_cast<uint32_t>(accBuildInfos.geometryInfos.size()),
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
		if (m_instances.size() == 0)
		{
			return;
		}

		vk::Device& device = Engine::GetRendererInstance()->GetDevice();

		delete[] m_tlasBuildInfo.rangeInfos;
		m_tlasBuildInfo.geometries.clear();
		m_tlasBuildInfo.buildSizes = vk::AccelerationStructureBuildSizesInfoKHR{};
		m_tlasBuildInfo.geometryInfo = vk::AccelerationStructureBuildGeometryInfoKHR{};

		{
			vk::DeviceOrHostAddressConstKHR instancesAddr;
			instancesAddr.setDeviceAddress(m_instancesBuffer->GetDeviceAddress());

			vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
			instancesData.setArrayOfPointers(VK_FALSE);
			instancesData.setData(instancesAddr);

			vk::AccelerationStructureGeometryDataKHR geometryData;
			geometryData.setInstances(instancesData);

			vk::AccelerationStructureGeometryKHR geometry;
			geometry.setGeometry(geometryData);
			geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);
			geometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
			m_tlasBuildInfo.geometries.push_back(geometry);
		}

		m_tlasBuildInfo.geometryInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild);
		m_tlasBuildInfo.geometryInfo.setGeometryCount(static_cast<uint32_t>(m_tlasBuildInfo.geometries.size()));
		m_tlasBuildInfo.geometryInfo.setPGeometries(m_tlasBuildInfo.geometries.data());
		m_tlasBuildInfo.geometryInfo.setDstAccelerationStructure(m_tlas.accelerationStructure);
		m_tlasBuildInfo.geometryInfo.setScratchData(m_tlasBuildInfo.scratchBuffer->GetDeviceAddress());

		{
			m_tlasBuildInfo.rangeInfos = new vk::AccelerationStructureBuildRangeInfoKHR[1];
			m_tlasBuildInfo.rangeInfos->setPrimitiveCount(static_cast<uint32_t>(m_instances.size()));
			m_tlasBuildInfo.rangeInfos->setFirstVertex(0);
			m_tlasBuildInfo.rangeInfos->setPrimitiveOffset(0);
			m_tlasBuildInfo.rangeInfos->setTransformOffset(0);
		}

		if (m_tlasBuildInfo.geometries.size() == 0)
		{
			return;
		}

		cmdBuff->buildAccelerationStructuresKHR(1, &m_tlasBuildInfo.geometryInfo, &m_tlasBuildInfo.rangeInfos);

		// waiting for tlas to build
		vk::MemoryBarrier barrier;
		barrier.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR);
		barrier.setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureReadKHR);
		cmdBuff->pipelineBarrier(
			vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
			vk::PipelineStageFlagBits::eRayTracingShaderKHR | vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlags(),
			1, &barrier,
			0, nullptr, 0, nullptr);
	}

	void RtScene::HandleUpdate(std::shared_ptr<GlobalUpdateMessage> msg)
	{

	}

	void RtScene::HandleFlip(std::shared_ptr<GlobalPostFrameMessage> msg)
	{
		m_frameIndexTruncated = (m_frameIndexTruncated + 1) % m_buildInfosArray.size();
		RTUtils::CleanupBuildInfos(m_buildInfosArray[m_frameIndexTruncated]);
	}

}

