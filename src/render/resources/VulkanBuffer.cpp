#include "VulkanBuffer.h"
#include "core/Engine.h"
#include "../TransferList.h"
#include "../Renderer.h"

namespace CGE
{
	namespace vk = VULKAN_HPP_NAMESPACE;

	VulkanBuffer::VulkanBuffer(bool inScoped, bool inCleanup)
		: m_scoped(inScoped)
		, m_cleanup(inCleanup)
	{
	}
	
	VulkanBuffer::~VulkanBuffer()
	{
		if (m_scoped)
		{
			Destroy();
		}
	}
	
	void VulkanBuffer::Create(bool deviceLocal/* = true*/)
	{
		if (m_buffer)
		{
			return;
		}
		m_vulkanDevice = &Engine::GetRendererInstance()->GetVulkanDevice();
		m_buffer = m_vulkanDevice->GetDevice().createBuffer(createInfo);
	
		m_descriptorInfo.setBuffer(m_buffer);
		m_descriptorInfo.setOffset(0);
		m_descriptorInfo.setRange(createInfo.size);

		BindMemory(deviceLocal ? vk::MemoryPropertyFlagBits::eDeviceLocal : vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	}
	
	void VulkanBuffer::CopyTo(DeviceSize inSize, const char* inData, bool pushToTransfer)
	{
		m_memRecord.pos.memory.MapCopyUnmap(MemoryMapFlags(), m_memRecord.pos.offset, inSize, inData, 0, inSize);
	}
	
	void VulkanBuffer::Destroy()
	{
		if (!m_cleanup)
		{
			return;
		}
		if (m_buffer)
		{
			m_vulkanDevice->GetDevice().destroyBuffer(m_buffer);
			m_buffer = nullptr;
			DeviceMemoryManager::GetInstance()->ReturnMemory(m_memRecord);
		}
	}
	
	void VulkanBuffer::BindMemory(MemoryPropertyFlags inMemPropertyFlags)
	{
		if (m_memRecord.pos.valid)
		{
			return;
		}
		DeviceMemoryManager* dmm = DeviceMemoryManager::GetInstance();
		m_memRecord = dmm->RequestMemory(GetMemoryRequirements(), inMemPropertyFlags);
		m_vulkanDevice->GetDevice().bindBufferMemory(m_buffer, m_memRecord.pos.memory, m_memRecord.pos.offset);
	}
	
	void VulkanBuffer::BindMemory(const DeviceMemory& inDeviceMemory, DeviceSize inMemOffset)
	{
		m_vulkanDevice->GetDevice().bindBufferMemory(m_buffer, inDeviceMemory, inMemOffset);
	}
	
	BufferCopy VulkanBuffer::CreateBufferCopy()
	{
		BufferCopy copyRegion;
		copyRegion.setSize(createInfo.size);
		copyRegion.setSrcOffset(0);
		copyRegion.setDstOffset(0);
	
		return copyRegion;
	}
	
	BufferMemoryBarrier VulkanBuffer::CreateMemoryBarrier(uint32_t inSrcQueue, uint32_t inDstQueue, AccessFlags inSrcAccessMask, AccessFlags inDstAccessMask)
	{
		BufferMemoryBarrier barrier;
	
		barrier.setBuffer(m_buffer);
		barrier.setSize(createInfo.size);
		barrier.setOffset(0);
		barrier.setSrcQueueFamilyIndex(inSrcQueue);
		barrier.setDstQueueFamilyIndex(inDstQueue);
		barrier.setSrcAccessMask(inSrcAccessMask);
		barrier.setDstAccessMask(inDstAccessMask);
	
		return barrier;
	}
	
	DescriptorBufferInfo& VulkanBuffer::GetDescriptorInfo()
	{
		return m_descriptorInfo;
	}
	
	DescriptorBufferInfo VulkanBuffer::GetDescriptorInfo() const
	{
		return m_descriptorInfo;
	}

	Buffer& VulkanBuffer::GetNativeBuffer()
	{
		return m_buffer;
	}
	
	Buffer VulkanBuffer::GetNativeBuffer() const
	{
		return m_buffer;
	}
	
	MemoryRequirements VulkanBuffer::GetMemoryRequirements() const
	{
		return m_vulkanDevice->GetDevice().getBufferMemoryRequirements(m_buffer);
	}
	
	MemoryRecord VulkanBuffer::GetMemoryRecord() const
	{
		return m_memRecord;
	}
	
	
	vk::DeviceAddress VulkanBuffer::GetDeviceAddress()
	{
		vk::BufferDeviceAddressInfo bufferInfo;
		bufferInfo.setBuffer(m_buffer);
		return m_vulkanDevice->GetDevice().getBufferAddress(bufferInfo);
	}

}
