#pragma once

#include <deque>
#include "../resources/DeviceMemoryWrapper.h"

class DeviceMemoryChunk
{
public:
	DeviceMemoryChunk();
	virtual ~DeviceMemoryChunk();

	void Allocate(uint32_t inElementCount, DeviceSize inSize, uint32_t inMemTypeBits, MemoryPropertyFlags inMemPropertyFlags);
	void Allocate(uint32_t inElementCount, const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags);
	void Free();

	DeviceSize AcquireSlot();
	void ReleaseSlot(DeviceSize inSlot);
	DeviceSize GetSlotOffset(DeviceSize inSlot);
	DeviceMemoryWrapper& GetMemory();
	bool HasFreeSpace();
protected:
	std::deque<DeviceSize> freeSlots;
	uint32_t numSlots;

	DeviceMemoryWrapper memory;
	DeviceSize elementSize;
};
