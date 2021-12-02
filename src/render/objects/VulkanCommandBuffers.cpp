#include "VulkanCommandBuffers.h"
#include "core/Engine.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::CommandPoolCreateInfo;
	using VULKAN_HPP_NAMESPACE::CommandPoolCreateFlagBits;
	using VULKAN_HPP_NAMESPACE::CommandBufferAllocateInfo;
	using VULKAN_HPP_NAMESPACE::CommandBufferLevel;

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
		m_transferPool = device->GetDevice().createCommandPool(poolInfo);
	
		CommandBufferAllocateInfo buffersInfo;
		buffersInfo.setCommandPool(m_transferPool);
		buffersInfo.setCommandBufferCount(cmdBuffersPerPool);
		buffersInfo.setLevel(CommandBufferLevel::ePrimary);
		m_transferBuffers = device->GetDevice().allocateCommandBuffers(buffersInfo);
	
		nextTransferBuffer = 0;
	
		//---------------------------------------------
	
		m_pools.resize(poolsCount);
		for (uint32_t index = 0; index < poolsCount; index++)
		{
			CommandPoolCreateInfo poolInfo;
			poolInfo.setQueueFamilyIndex(device->GetPhysicalDevice().GetCachedQueueFamiliesIndices().graphicsFamily.value());
			poolInfo.setFlags(CommandPoolCreateFlagBits::eResetCommandBuffer);
			m_pools[index] = device->GetDevice().createCommandPool(poolInfo);
	
			CommandBufferAllocateInfo buffersInfo;
			buffersInfo.setCommandPool(m_pools[index]);
			buffersInfo.setCommandBufferCount(cmdBuffersPerPool);
			buffersInfo.setLevel(CommandBufferLevel::ePrimary);
			m_buffers[index] = device->GetDevice().allocateCommandBuffers(buffersInfo);
		}
	
		nextBuffer = std::vector<uint32_t>(poolsCount, 0);
	}
	
	void VulkanCommandBuffers::Destroy()
	{
		if (device)
		{
			device->GetDevice().freeCommandBuffers(m_transferPool, cmdBuffersPerPool, m_transferBuffers.data());
			device->GetDevice().destroyCommandPool(m_transferPool);
	
			for (uint32_t index = 0; index < poolsCount; index++)
			{
				device->GetDevice().freeCommandBuffers(m_pools[index], cmdBuffersPerPool, m_buffers[index].data());
				device->GetDevice().destroyCommandPool(m_pools[index]);
			}
		}
	}
	
	CommandPool& VulkanCommandBuffers::GetCommandPool(uint32_t inPoolIndex)
	{
		return m_pools[inPoolIndex];
	}
	
	CommandBuffer& VulkanCommandBuffers::GetBufferForPool(uint32_t inPoolIndex, uint32_t inBufferIndex)
	{
		return m_buffers[inPoolIndex][inBufferIndex];
	}
	
	CommandBuffer& VulkanCommandBuffers::GetBufferForFrame()
	{
		uint32_t poolIndex = Engine::GetFrameIndex(m_pools.size());
		return m_buffers[poolIndex][0]; // leave 0 for now, not sure if more buffers needed
	}

	CommandPool& VulkanCommandBuffers::GetTransferPool()
	{
		return m_transferPool;
	}
	
	CommandBuffer& VulkanCommandBuffers::GetTransferBufferForFrame()
	{
		return m_transferBuffers[Engine::GetFrameIndex(m_transferBuffers.size())];
	}
	
}
