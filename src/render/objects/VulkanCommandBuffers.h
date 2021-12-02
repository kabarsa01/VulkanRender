#pragma once

#include "vulkan/vulkan.hpp"
#include "VulkanDevice.h"
#include <vector>
#include <map>

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::CommandPool;
	using VULKAN_HPP_NAMESPACE::CommandBuffer;
	
	class VulkanCommandBuffers
	{
	public:
		VulkanCommandBuffers();
		virtual ~VulkanCommandBuffers();
	
		void Create(VulkanDevice* inDevice, uint32_t inPoolsCount, uint32_t inCmdBuffersPerPool);
		void Destroy();
	
		CommandPool& GetCommandPool(uint32_t inPoolIndex);
		CommandBuffer& GetBufferForPool(uint32_t inPoolIndex, uint32_t inBufferIndex);
		CommandBuffer& GetBufferForFrame();
	
		CommandPool& GetTransferPool();
		CommandBuffer& GetTransferBufferForFrame();
	private:
		VulkanDevice* device;
		std::vector<CommandPool> m_pools;
		std::map<uint32_t, std::vector<CommandBuffer>> m_buffers;
		std::vector<uint32_t> nextBuffer;
	
		CommandPool m_transferPool;
		std::vector<CommandBuffer> m_transferBuffers;
		uint32_t nextTransferBuffer;
	
		uint32_t poolsCount;
		uint32_t cmdBuffersPerPool;
	};
	
}
