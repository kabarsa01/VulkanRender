#ifndef __RT_UTILS_H__
#define __RT_UTILS_H__

#include "vulkan/vulkan.hpp"

#include "data/MeshData.h"
#include "scene/mesh/MeshComponent.h"
#include "data/BufferData.h"

namespace CGE
{

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

	struct AccelStructure
	{
		vk::AccelerationStructureKHR accelerationStructure;
		BufferDataPtr buffer;
	};

	//-------------------------------------------------------------------------------------

	struct AccelStructureBuildInfo
	{
		vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
		std::vector<vk::AccelerationStructureGeometryKHR> geometries;
		vk::AccelerationStructureBuildRangeInfoKHR* rangeInfos;
		vk::AccelerationStructureBuildSizesInfoKHR buildSizes;
		BufferDataPtr scratchBuffer;
	};

	//-------------------------------------------------------------------------------------

	struct AccelStructuresBuildInfos
	{
		std::vector<vk::AccelerationStructureBuildGeometryInfoKHR> geometryInfos;
		std::vector<vk::AccelerationStructureGeometryKHR*> geometries;
		std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> rangeInfos;
		std::vector<vk::AccelerationStructureBuildSizesInfoKHR> buildSizes;
		std::vector<BufferDataPtr> scratchBuffers;
	};

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

	class RTUtils
	{
	public:
		static vk::AccelerationStructureGeometryTrianglesDataKHR GetGeometryTrianglesData(MeshDataPtr meshData);
		static vk::AccelerationStructureGeometryInstancesDataKHR GetGeometryInstancesData(MeshDataPtr meshData);
		static vk::AccelerationStructureInstanceKHR GetAccelerationStructureInstance(MeshComponentPtr meshComp);

		static void CleanupAccelerationStructure(AccelStructure& accelStructure);
		static void CleanupBuildInfo(AccelStructureBuildInfo& buildInfo);
		static void CleanupBuildInfos(AccelStructuresBuildInfos& buildInfos);
	};

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

}

#endif

