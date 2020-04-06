#pragma once

#include <deque>
#include "../resources/DeviceMemoryWrapper.h"

static const uint64_t MEM_LAYERS_COUNT = 15;
static const uint64_t MEM_SEGMENTS_COUNT = 1 << (MEM_LAYERS_COUNT - 1);
static const uint64_t MEM_TREE_SIZE = (1 << MEM_LAYERS_COUNT) - 1;

struct MemoryChunkPosition
{
	bool valid;
	uint32_t layer;
	uint32_t index;
	DeviceSize offset;
	DeviceMemoryWrapper memory;
};

class DeviceMemoryChunk
{
public:
	DeviceMemoryChunk(DeviceSize inSegmentSize);
	virtual ~DeviceMemoryChunk();

	void Allocate(uint32_t inMemTypeBits, MemoryPropertyFlags inMemPropertyFlags);
	void Allocate(const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags);
	void Free();

	MemoryChunkPosition AcquireSegment(DeviceSize inSize);

	DeviceSize AcquireSlot();
	void ReleaseSlot(DeviceSize inSlot);
	DeviceSize GetSlotOffset(DeviceSize inSlot);
	DeviceMemoryWrapper& GetMemory();
	bool HasFreeSpace();
protected:
	// flattened binary tree for memory segment tracking
	unsigned char memoryTree[MEM_TREE_SIZE];
	DeviceMemoryWrapper memory;
	DeviceSize segmentSize;

	uint32_t GetLayerStartIndex(uint32_t inLayer);
	uint32_t GetParentIndex(uint32_t inIndex);
	uint32_t GetChildIndex(uint32_t inIndex);
	uint32_t GetSiblingIndex(uint32_t inIndex);
	DeviceSize CalculateOffset(uint32_t inLayer, uint32_t inIndex);

	bool FindSegmentIndex(uint32_t inStartLayer, uint32_t inTargetLayer, uint32_t inIndex, uint32_t& outTargetIndex);
	void MarkFreeUp(uint32_t inStartLayer, uint32_t inIndex);
	void MarkNotFreeUp(uint32_t inStartLayer, uint32_t inIndex);
};
