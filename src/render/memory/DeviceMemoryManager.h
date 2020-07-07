#pragma once

#include "vulkan/vulkan.hpp"
#include <chrono>
#include <map>
#include <vector>
#include "DeviceMemoryChunk.h"

struct MemoryRecord
{
	uint64_t regionHash;
	uint64_t chunkIndex;
	MemoryPosition pos;
};

// 2 to 11th is 2048, so we will have a tree with 2048 segments at layer 0, it's tree's leaf nodes
// it's a binary tree with layer 0 being the leaf nodes, and leaf nodes being the smallest memory
// segments available to occupy
static const uint32_t memoryTreeDepth = 12;
// equals to multiplying by 8, it means we will use only first 4 layers of the tree for data storage
// all other layers will be used for traversal speedup
static const uint32_t sizeRangeShift = 3;
// we limit ourselves on 4th range of memory. if we use our basic chunk at 64 byte granularity, we will have the following ranges
// ---------------------------------------------------------------------------------------
// let the sizeRangeShift be 3 and memoryTreeDepth 12, which will give us 2048 leaf nodes
// ---------------------------------------------------------------------------------------
//
// 64 << sizeRangeShift = 512          :: 64 * 2048     = 128K chunk
// 1024 << sizeRangeShift = 8K         :: 1024 * 2048   = 2M chunk
// 16K << sizeRangeShift = 128K        :: 16K * 2048    = 32M chunk
// 256K << sizeRangeShift = 2M         :: 256K * 2048   = 512M chunk
//
// it means that range 4 will allocate 512M chunks
// in case of shift of 4 we will have corresponding granularities and chunk sizes for tree depth 12
//
// 64   -   128K
// 2K   -   4M
// 64K  -   128M
// 2M   -   4GB
// 
static const uint32_t maxRanges = 4;
// base memory segment size, the smallest you can allocate
static const DeviceSize baseMemorySegmentSize = 64;

class DeviceMemoryManager
{
public:
	static DeviceMemoryManager* GetInstance();

	MemoryRecord RequestMemory(const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags);
	void ReturnMemory(const MemoryRecord& inMemoryRecord);
	void CleanupMemory();

	DeviceMemoryChunk* GetMemoryChunk(MemoryRecord inMemPosition);
protected:
	static DeviceMemoryManager* staticInstance;

	std::map<uint64_t, std::vector<DeviceMemoryChunk*>> memRegions;

	DeviceMemoryManager();
	DeviceMemoryManager(const DeviceMemoryManager&) {}
	void operator= (const DeviceMemoryManager&) {}
	virtual ~DeviceMemoryManager();

	DeviceSize GetRangeBase(uint32_t inIndex);
	DeviceSize GetRangeMax(uint32_t inIndex);
	uint32_t GetRangeIndex(DeviceSize inSize);
};
