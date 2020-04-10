#include "VulkanCommandBuffers.h"

VulkanCommandBuffers::VulkanCommandBuffers()
{

}

VulkanCommandBuffers::~VulkanCommandBuffers()
{

}

void VulkanCommandBuffers::Create(VulkanDevice* inDevice, uint32_t inPoolsCount, uint32_t inCmdBuffersPerPool)
{
	device = inDevice;
	poolsCount = inPoolsCount;
	cmdBuffersPerPool = inCmdBuffersPerPool;

	//---------------------------------------------
	// making separate transfer pool just because why not

	CommandPoolCreateInfo poolInfo;
	poolInfo.setQueueFamilyIndex(device->GetPhysicalDevice().GetCachedQueueFamiliesIndices().transferFamily.value());
	poolInfo.setFlags(CommandPoolCreateFlagBits::eResetCommandBuffer);
	transferPool = device->GetDevice().createCommandPool(poolInfo);

	CommandBufferAllocateInfo buffersInfo;
	buffersInfo.setCommandPool(transferPool);
	buffersInfo.setCommandBufferCount(cmdBuffersPerPool);
	buffersInfo.setLevel(CommandBufferLevel::ePrimary);
	transferBuffers = device->GetDevice().allocateCommandBuffers(buffersInfo);

	nextTransferBuffer = 0;

	//---------------------------------------------

	pools.resize(poolsCount);
	for (uint32_t index = 0; index < poolsCount; index++)
	{
		CommandPoolCreateInfo poolInfo;
		poolInfo.setQueueFamilyIndex(device->GetPhysicalDevice().GetCachedQueueFamiliesIndices().graphicsFamily.value());
		poolInfo.setFlags(CommandPoolCreateFlagBits::eResetCommandBuffer);
		pools[index] = device->GetDevice().createCommandPool(poolInfo);

		CommandBufferAllocateInfo buffersInfo;
		buffersInfo.setCommandPool(pools[index]);
		buffersInfo.setCommandBufferCount(cmdBuffersPerPool);
		buffersInfo.setLevel(CommandBufferLevel::ePrimary);
		buffers[index] = device->GetDevice().allocateCommandBuffers(buffersInfo);
	}

	nextBuffer = std::vector<uint32_t>(poolsCount, 0);
}

void VulkanCommandBuffers::Destroy()
{
	if (device)
	{
		device->GetDevice().freeCommandBuffers(transferPool, cmdBuffersPerPool, transferBuffers.data());
		device->GetDevice().destroyCommandPool(transferPool);

		for (uint32_t index = 0; index < poolsCount; index++)
		{
			device->GetDevice().freeCommandBuffers(pools[index], cmdBuffersPerPool, buffers[index].data());
			device->GetDevice().destroyCommandPool(pools[index]);
		}
	}
}

CommandPool& VulkanCommandBuffers::GetCommandPool(uint32_t inPoolIndex)
{
	return pools[inPoolIndex];
}

CommandBuffer& VulkanCommandBuffers::GetNextForPool(uint32_t inPoolIndex)
{
	uint32_t bufferIndex = nextBuffer[inPoolIndex];
	nextBuffer[inPoolIndex] = (bufferIndex + 1) % cmdBuffersPerPool;
	return buffers[inPoolIndex][bufferIndex];
}

CommandBuffer& VulkanCommandBuffers::GetForPool(uint32_t inPoolIndex, uint32_t inBufferIndex)
{
	return buffers[inPoolIndex][inBufferIndex];
}

CommandPool& VulkanCommandBuffers::GetTransferPool()
{
	return transferPool;
}

CommandBuffer& VulkanCommandBuffers::GetNextTransferBuffer()
{
	uint32_t bufferIndex = nextTransferBuffer;
	nextTransferBuffer = (bufferIndex + 1) % cmdBuffersPerPool;
	return transferBuffers[bufferIndex];
}

