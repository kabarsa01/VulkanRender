#pragma once

#include "vulkan/vulkan.hpp"

using namespace VULKAN_HPP_NAMESPACE;

class DeviceMemoryWrapper
{
public:
	DeviceMemoryWrapper();
	DeviceMemoryWrapper(const MemoryPropertyFlags& inMemPropertyFlags);
	virtual ~DeviceMemoryWrapper();

	inline void SetSize(DeviceSize inSize) { size = inSize; }
	inline DeviceSize GetSize() { return size; }
	inline void SetMemTypeBits(uint32_t inMemTypeBits) { memTypeBits = inMemTypeBits; }
	inline uint32_t GetMemTypeBits() { return memTypeBits; }
	void SetMemPropertyFlags(MemoryPropertyFlags inMemPropertyFlags);
	MemoryPropertyFlags GetMemPropertyFlags();

	void Allocate(const MemoryRequirements& inMemRequirements);
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

	static uint32_t FindMemoryType(uint32_t inTypeFilter, MemoryPropertyFlags inPropFlags);
protected:
	Device device;
	DeviceMemory deviceMemory;
	DeviceSize size;
	uint32_t memTypeBits;
	MemoryPropertyFlags memPropertyFlags;
	void* mappedMem;

	void AllocateInternal(DeviceSize inSize, uint32_t inMemTypeBits);
};


