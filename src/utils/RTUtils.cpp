#include "RTUtils.h"
#include "scene/SceneObjectComponent.h"
#include "scene/SceneObjectBase.h"
#include "scene/Transform.h"

namespace CGE
{

	vk::AccelerationStructureGeometryTrianglesDataKHR RTUtils::GetGeometryTrianglesData(MeshDataPtr meshData)
	{
		vk::AccelerationStructureGeometryTrianglesDataKHR trisData;
		trisData.setVertexFormat(vk::Format::eR32G32B32Sfloat);
		trisData.setVertexStride(sizeof(Vertex));
		trisData.setVertexData(meshData->GetVertexBuffer()->GetDeviceAddress());
		trisData.setMaxVertex(meshData->GetVertexCount());
		trisData.setIndexType(vk::IndexType::eUint32);
		trisData.setIndexData(meshData->GetIndexBuffer()->GetDeviceAddress());
		trisData.setTransformData({}); // identity

		return trisData;
	}

	vk::AccelerationStructureGeometryInstancesDataKHR RTUtils::GetGeometryInstancesData(MeshDataPtr meshData)
	{
		vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
		//instancesData.set

		return instancesData;
	}

	vk::AccelerationStructureInstanceKHR RTUtils::GetAccelerationStructureInstance(MeshComponentPtr meshComp)
	{
		vk::AccelerationStructureInstanceKHR instance;

		glm::mat4x4 mat = meshComp->GetParent()->transform.GetMatrix();
		mat = glm::transpose(mat);
		memcpy(&instance.transform, &mat, sizeof(instance.transform));

		instance.setFlags(vk::GeometryInstanceFlagBitsKHR::eForceOpaque);
		instance.setMask(0xFFFFFFFF);

		return instance;
	}

	void RTUtils::CleanupAccelerationStructure(AccelStructure& accelStructure)
	{
		if (accelStructure.accelerationStructure)
		{
			Engine::GetRendererInstance()->GetDevice().destroyAccelerationStructureKHR(accelStructure.accelerationStructure);
		}
		accelStructure.buffer = nullptr;
	}

	void RTUtils::CleanupBuildInfo(AccelStructureBuildInfo& buildInfo)
	{
		delete[] buildInfo.rangeInfos;
		buildInfo.geometries.clear();
		buildInfo.scratchBuffer = nullptr;

		buildInfo.buildSizes = vk::AccelerationStructureBuildSizesInfoKHR{};
		buildInfo.geometryInfo = vk::AccelerationStructureBuildGeometryInfoKHR{};
	}

	void RTUtils::CleanupBuildInfos(AccelStructuresBuildInfos& buildInfos)
	{
		for (uint32_t idx = 0; idx < buildInfos.geometryInfos.size(); idx++)
		{
			delete[] buildInfos.geometries[idx];
			delete[] buildInfos.rangeInfos[idx];
			buildInfos.scratchBuffers[idx] = nullptr;
		}

		buildInfos.buildSizes.clear();
		buildInfos.geometries.clear();
		buildInfos.geometryInfos.clear();
		buildInfos.rangeInfos.clear();
		buildInfos.scratchBuffers.clear();
	}

	vk::Pipeline RTUtils::CreatePipeline(vk::PipelineLayout layout, ShaderBindingTable& sbt)
	{
		vk::RayTracingPipelineCreateInfoKHR pipelineInfo;
		pipelineInfo.setGroupCount(static_cast<uint32_t>(sbt.GetShaderGroups().size()));
		pipelineInfo.setPGroups(sbt.GetShaderGroups().data());
		pipelineInfo.setStageCount(static_cast<uint32_t>(sbt.GetShaderStages().size()));
		pipelineInfo.setPStages(sbt.GetShaderStages().data());
		pipelineInfo.setFlags({});
		pipelineInfo.setLayout(layout);

		auto pipelineResult = Engine::GetRendererInstance()->GetDevice().createRayTracingPipelineKHR(nullptr, nullptr, pipelineInfo);
		if (pipelineResult.result != vk::Result::eSuccess)
		{
			return nullptr;
		}
		return pipelineResult.value;
	}

}

