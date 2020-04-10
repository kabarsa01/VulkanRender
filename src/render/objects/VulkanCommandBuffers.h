#pragma once

#include "vulkan/vulkan.hpp"
#include "VulkanDevice.h"
#include <vector>
#include <map>

using namespace VULKAN_HPP_NAMESPACE;

class VulkanCommandBuffers
{
public:
	VulkanCommandBuffers();
	virtual ~VulkanCommandBuffers();

	void Create(VulkanDevice* inDevice, uint32_t inPoolsCount, uint32_t inCmdBuffersPerPool);
	void Destroy();

	CommandPool& GetCommandPool(uint32_t inPoolIndex);
	CommandBuffer& GetNextForPool(uint32_t inPoolIndex);
	CommandBuffer& GetForPool(uint32_t inPoolIndex, uint32_t inBufferIndex);

	CommandPool& GetTransferPool();
	CommandBuffer& GetNextTransferBuffer();
private:
	VulkanDevice* device;
	std::vector<CommandPool> pools;
	std::map<uint32_t, std::vector<CommandBuffer>> buffers;
	std::vector<uint32_t> nextBuffer;

	CommandPool transferPool;
	std::vector<CommandBuffer> transferBuffers;
	uint32_t nextTransferBuffer;

	uint32_t poolsCount;
	uint32_t cmdBuffersPerPool;
};

