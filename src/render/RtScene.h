#ifndef __RT_SCENE_H__
#define __RT_SCENE_H__

#include <vector>
#include <vulkan/vulkan.hpp>

#include "messages/MessageSubscriber.h"
#include "resources/VulkanBuffer.h"
#include "common/HashString.h"

namespace CGE
{

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

	struct AccelStructure
	{
		vk::AccelerationStructureKHR accelerationStructure;
		VulkanBuffer buffer;
	};

	struct AccelStructureBuildInfo
	{
		vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
		std::vector<vk::AccelerationStructureGeometryKHR> geometries;
		std::vector<vk::AccelerationStructureBuildRangeInfoKHR> rangeInfos;
		vk::AccelerationStructureBuildSizesInfoKHR buildSizes;
		VulkanBuffer scratchBuffer;
	};

	struct AccelStructuresBuildInfos
	{
		std::vector<vk::AccelerationStructureBuildGeometryInfoKHR> geometryInfos;
		std::vector<vk::AccelerationStructureGeometryKHR*> geometries;
		std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> rangeInfos;
		std::vector<vk::AccelerationStructureBuildSizesInfoKHR> buildSizes;
		std::vector<VulkanBuffer> scratchBuffers;
	};

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

	// Entity to manage scene representation for RT, updating BLAS's and TLAS as well as 
	// book keeping when scene changes
	class RtScene
	{
	public:
		RtScene();
		~RtScene();

		void BuildMeshBlases(vk::CommandBuffer* cmdBuff);
		void BuildSceneTlas(vk::CommandBuffer* cmdBuff);
	private:
		MessageSubscriber m_messageSubscriber;
		std::unordered_map<HashString, AccelStructure> m_blasTable;
		AccelStructure m_tlas;

		uint32_t m_frameIndexTruncated;
		std::array<AccelStructuresBuildInfos, 3> m_buildInfosArray; // TODO do something with multi buffering

		void CleanupBuildInfos(AccelStructuresBuildInfos& buildInfos);

		void HandleUpdate(std::shared_ptr<GlobalUpdateMessage> msg);
		void HandleFlip(std::shared_ptr<GlobalFlipMessage> msg);
	};

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

}

#endif
