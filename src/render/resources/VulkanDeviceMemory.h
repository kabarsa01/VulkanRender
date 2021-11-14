#pragma once

#include "vulkan/vulkan.hpp"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::Device;
	using VULKAN_HPP_NAMESPACE::DeviceSize;
	using VULKAN_HPP_NAMESPACE::DeviceMemory;
	using VULKAN_HPP_NAMESPACE::MemoryPropertyFlags;
	using VULKAN_HPP_NAMESPACE::MemoryPropertyFlagBits;
	using VULKAN_HPP_NAMESPACE::MemoryRequirements;
	using VULKAN_HPP_NAMESPACE::MemoryMapFlags;
	using VULKAN_HPP_NAMESPACE::MemoryMapFlagBits;

	class VulkanDeviceMemory
	{
	public:
		VulkanDeviceMemory(bool inScoped = false);
		virtual ~VulkanDeviceMemory();
	
		VulkanDeviceMemory& SetSize(DeviceSize inSize);
		DeviceSize GetSize() { return m_size; }
		VulkanDeviceMemory& SetRequirements(const vk::MemoryRequirements& memRequirements);
		vk::MemoryRequirements GetRequirements() { return m_requirements; }
		VulkanDeviceMemory& SetPropertyFlags(MemoryPropertyFlags inMemPropertyFlags);
		vk::MemoryPropertyFlags GetPropertyFlags() { return m_propertyFlags; }
	
		void Allocate();
		void Free();
		bool IsValid();
	
		void* MapMemory(MemoryMapFlags inMapFlags, DeviceSize inMappingOffset, DeviceSize inMappingSize);
		void* GetMappedMem();
		void UnmapMemory();
		void CopyToMappedMem(size_t inDstOffset, const void* inSrcData, size_t inSrcOffset, size_t inSize);
		void MapCopyUnmap(
			MemoryMapFlags inMapFlags, DeviceSize inMappingOffset, 
			DeviceSize inMappingSize, const void* inSrcData, 
			size_t inCopyOffset, size_t inCopySize);
	
		operator bool() const;
		operator DeviceMemory() const;
	
		static uint32_t FindMemoryTypeStatic(uint32_t inTypeFilter, MemoryPropertyFlags inPropFlags);
	protected:
		bool m_deviceLocal = true;
		bool m_scoped = false;
		Device m_nativeDevice;
		DeviceMemory m_deviceMemory;
		DeviceSize m_size;
		vk::MemoryPropertyFlags m_propertyFlags;
		vk::MemoryRequirements m_requirements;
		void* m_mappedMem;
	};
	
	
}
