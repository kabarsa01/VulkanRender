#ifndef __RT_SCENE_H__
#define __RT_SCENE_H__

#include <vector>
#include <array>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "messages/MessageSubscriber.h"
#include "resources/VulkanBuffer.h"
#include "common/HashString.h"
#include "utils/RTUtils.h"
#include "shader/RtShader.h"

namespace CGE
{

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

		void Init();

		void UpdateShaders();
		void UpdateInstances();
		void BuildMeshBlases(vk::CommandBuffer* cmdBuff);
		void BuildSceneTlas(vk::CommandBuffer* cmdBuff);
	private:
		MessageSubscriber m_messageSubscriber;
		std::unordered_map<HashString, AccelStructure> m_blasTable;
		AccelStructure m_tlas;
		// shaders data
		std::vector<RtShaderPtr> m_shaders;
		std::vector<vk::PipelineShaderStageCreateInfo> m_stages;
		std::unordered_map<HashString, uint32_t> m_shaderIndices;
		std::array<std::vector<RtShaderPtr>, static_cast<uint32_t>(ERtShaderType::RST_MAX)> m_shadersByType;
		// groups data
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_groups;
		std::unordered_map<HashString, uint32_t> m_materialGroupIndices;
		uint32_t m_missGroupsOffset;
		uint32_t m_hitGroupsOffset;

		std::vector<vk::AccelerationStructureInstanceKHR> m_instances;
		uint32_t m_frameIndexTruncated;
		std::array<AccelStructuresBuildInfos, 3> m_buildInfosArray; // TODO do something with multi buffering

		VulkanBuffer m_instancesBuffer;

		void CleanupBuildInfos(AccelStructuresBuildInfos& buildInfos);
		void FillGeneralShaderGroups(const std::vector<RtShaderPtr>& shaders, std::vector<vk::RayTracingShaderGroupCreateInfoKHR>& groups);

		void HandleUpdate(std::shared_ptr<GlobalUpdateMessage> msg);
		void HandleFlip(std::shared_ptr<GlobalFlipMessage> msg);
	};

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

}

#endif
