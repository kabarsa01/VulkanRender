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
		VulkanDeviceMemory(const MemoryPropertyFlags& inMemPropertyFlags, bool inScoped = false);
		virtual ~VulkanDeviceMemory();
	
		VulkanDeviceMemory& SetSize(DeviceSize inSize);
		DeviceSize GetSize() { return size; }
		VulkanDeviceMemory& SetMemTypeBits(uint32_t inMemTypeBits);
		uint32_t GetMemTypeBits() { return memTypeBits; }
		VulkanDeviceMemory& SetMemPropertyFlags(MemoryPropertyFlags inMemPropertyFlags);
		MemoryPropertyFlags GetMemPropertyFlags();
	
		void Allocate(DeviceSize inSize, uint32_t inMemTypeBits, MemoryPropertyFlags inMemPropertyFlags);
		void Allocate(const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags);
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
		bool scoped = false;
		Device device;
		DeviceMemory deviceMemory;
		DeviceSize size;
		uint32_t memTypeBits;
		MemoryPropertyFlags memPropertyFlags;
		void* mappedMem;
	};
	
	
}
