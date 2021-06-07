#ifndef __RT_SCENE_H__
#define __RT_SCENE_H__

#include <vector>
#include <vulkan/vulkan.hpp>

#include "messages/MessageSubscriber.h"
#include "resources/VulkanBuffer.h"
#include "common/HashString.h"

namespace vk = VULKAN_HPP_NAMESPACE;

namespace CGE
{

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

	struct AccelStructure
	{
		vk::AccelerationStructureKHR accelStr;
		VulkanBuffer buffer;
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
	private:
		MessageSubscriber m_messageSubscriber;
		std::unordered_map<HashString, AccelStructure> m_blasTable;
		AccelStructure m_tlas;

		void HandleUpdate(std::shared_ptr<GlobalUpdateMessage> msg);
	};

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

}

#endif
