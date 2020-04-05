#pragma once

#include "vulkan/vulkan.hpp"
#include <map>
#include <vector>
#include "DeviceMemoryChunk.h"

struct MemoryRecord
{
	uint64_t regionHash;
	uint64_t chunkIndex;
	DeviceSize chunkSlot;
	DeviceSize memoryOffset;
	DeviceMemoryWrapper memory;
};

class DeviceMemoryManager
{
public:
	static DeviceMemoryManager* GetInstance();

	MemoryRecord RequestMemory(const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags);
	void ReturnMemory(const MemoryRecord& inMemoryPosition);
	void CleanupMemory();

	DeviceMemoryChunk& GetMemoryChunk(MemoryRecord inMemPosition);
protected:
	static DeviceMemoryManager staticInstance;

	std::map<uint64_t, std::vector<DeviceMemoryChunk>> memRegions;

	DeviceMemoryManager();
	virtual ~DeviceMemoryManager();
};
