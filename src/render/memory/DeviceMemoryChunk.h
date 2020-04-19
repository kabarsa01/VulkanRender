#pragma once

#include <deque>
#include "../resources/VulkanDeviceMemory.h"

struct MemoryPosition
{
	bool valid = false;
	uint32_t layer;
	uint32_t index;
	DeviceSize offset;
	VulkanDeviceMemory memory;

	MemoryPosition()
		: valid(false)
		, layer(-1)
		, index(-1)
		, offset(-1)
		, memory()
	{}
};

class DeviceMemoryChunk
{
public:
	DeviceMemoryChunk(DeviceSize inSegmentSize, uint32_t inTreeDepth);
	DeviceMemoryChunk(const DeviceMemoryChunk& inOther);
	virtual ~DeviceMemoryChunk();

	void Allocate(uint32_t inMemTypeBits, MemoryPropertyFlags inMemPropertyFlags);
	void Allocate(const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags);
	void Free();

	MemoryPosition AcquireSegment(DeviceSize inSize);
	void ReleaseSegment(const MemoryPosition& inMemoryPosition);

	VulkanDeviceMemory& GetMemory();
	bool HasFreeSpace();
protected:
	uint32_t treeDepth;
	uint32_t treeSize;
	uint32_t segmentCount;
	// flattened binary tree for memory segment tracking
	unsigned char* memoryTree;
	VulkanDeviceMemory memory;
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
