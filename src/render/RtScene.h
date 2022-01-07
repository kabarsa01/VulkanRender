#ifndef __RT_SCENE_H__
#define __RT_SCENE_H__

#include <vector>
#include <array>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "messages/MessageSubscriber.h"
#include "common/HashString.h"
#include "utils/RTUtils.h"
#include "shader/RtShader.h"
#include "shader/ShaderBindingTable.h"

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
		void Cleanup();

		void UpdateShaders();
		void UpdateInstances();
		void BuildMeshBlases(vk::CommandBuffer* cmdBuff);
		void BuildSceneTlas(vk::CommandBuffer* cmdBuff);

		AccelStructure& GetTlas() { return m_tlas; }
		ShaderBindingTable& GetGlobalShaderBindingTable() { return m_shaderBindingTables[Engine::GetFrameIndex(m_shaderBindingTables.size())]; }
	private:
		MessageSubscriber m_messageSubscriber;
		std::unordered_map<HashString, AccelStructure> m_blasTable;
		AccelStructure m_tlas;
		AccelStructureBuildInfo m_tlasBuildInfo;
		// shaders data
		std::vector<ShaderBindingTable> m_shaderBindingTables;

		std::vector<vk::AccelerationStructureInstanceKHR> m_instances;
		BufferDataPtr m_instancesBuffer;
		uint32_t m_frameIndexTruncated;
		std::array<AccelStructuresBuildInfos, 3> m_buildInfosArray; // TODO do something with multi buffering

		void HandleUpdate(std::shared_ptr<GlobalUpdateMessage> msg);
		void HandleFlip(std::shared_ptr<GlobalPostFrameMessage> msg);
	};

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

}

#endif
