#pragma once

#include "vulkan/vulkan.hpp"
#include "../memory/DeviceMemoryManager.h"
#include "../objects/VulkanDevice.h"
#include <vector>

namespace CGE
{

	using VULKAN_HPP_NAMESPACE::DeviceSize;
	using VULKAN_HPP_NAMESPACE::MemoryPropertyFlags;
	using VULKAN_HPP_NAMESPACE::MemoryRequirements;
	using VULKAN_HPP_NAMESPACE::AccessFlags;
	using VULKAN_HPP_NAMESPACE::Buffer;
	using VULKAN_HPP_NAMESPACE::BufferCopy;
	using VULKAN_HPP_NAMESPACE::BufferUsageFlags;
	using VULKAN_HPP_NAMESPACE::BufferCreateInfo;
	using VULKAN_HPP_NAMESPACE::BufferMemoryBarrier;
	using VULKAN_HPP_NAMESPACE::DescriptorBufferInfo;

	class VulkanBuffer
	{
	public:
		BufferCreateInfo createInfo;
	
		VulkanBuffer(bool inScoped = false, bool inCleanup = true);
		virtual ~VulkanBuffer();
	
		void Create(bool deviceLocal = true);
		void Destroy();
	
		void CopyTo(DeviceSize inSize, const char* inData, bool pushToTransfer = true);
	
		BufferCopy CreateBufferCopy();
		BufferMemoryBarrier CreateMemoryBarrier(uint32_t inSrcQueue, uint32_t inDstQueue, AccessFlags inSrcAccessMask, AccessFlags inDstAccessMask);
		DescriptorBufferInfo& GetDescriptorInfo();
		DescriptorBufferInfo GetDescriptorInfo() const;
	
		VulkanBuffer* GetStagingBuffer() { return m_stagingBuffer; }
		Buffer& GetNativeBuffer();
		Buffer GetNativeBuffer() const;
		MemoryRequirements GetMemoryRequirements();
		MemoryRecord& GetMemoryRecord();
		vk::DeviceAddress GetDeviceAddress();
	
		operator Buffer() const { return m_buffer; }
		operator bool() const { return m_buffer; }
	
		void SetCleanup(bool inCleanup) { m_cleanup = inCleanup; }
	protected:
		VulkanDevice* m_vulkanDevice;
		Buffer m_buffer;
		DescriptorBufferInfo m_descriptorInfo;
		MemoryRecord m_memRecord;
	
		bool m_scoped = false;
		bool m_cleanup = true;
		bool m_deviceLocal = true;
	
		VulkanBuffer* m_stagingBuffer;

		void BindMemory(MemoryPropertyFlags inMemPropertyFlags);
		void BindMemory(const DeviceMemory& inDeviceMemory, DeviceSize inMemOffset);
		VulkanBuffer* CreateStagingBuffer();
		VulkanBuffer* CreateStagingBuffer(DeviceSize inSize, const char* inData);
	};
}
