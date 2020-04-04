#pragma once

#include "vulkan\vulkan.hpp"

using namespace VULKAN_HPP_NAMESPACE;

class MemoryBuffer
{
public:
	MemoryBuffer();
	virtual ~MemoryBuffer();

	Buffer GetBuffer();
	DeviceMemory GetDeviceMemory();

	void SetSize(uint32_t inSize) { size = inSize; }
	uint32_t GetSize() { return size; }
	void SetUsage(BufferUsageFlags inUsageFlags) { usageFlags = inUsageFlags; }
	void SetMemProperty(MemoryPropertyFlags inMemPropertyFlags) { memPropertyFlags = inMemPropertyFlags; }

	void Create();
	void Destroy();
	void* MapMemory(MemoryMapFlags inMapFlags, DeviceSize inOffset);
	void UnmapMemory();
	void CopyData(const void* inSrcData, MemoryMapFlags inMapFlags, DeviceSize inOffset);

	static void CopyBuffer(MemoryBuffer& inSrc, MemoryBuffer& inDst);
	static uint32_t FindMemoryType(uint32_t inTypeFilter, MemoryPropertyFlags inPropFlags);
protected:
	Buffer buffer;
	DeviceMemory deviceMemory;

	uint32_t size;
	BufferUsageFlags usageFlags;
	MemoryPropertyFlags memPropertyFlags;
};
