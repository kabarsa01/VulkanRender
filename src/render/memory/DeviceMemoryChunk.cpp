#include "DeviceMemoryChunk.h"

DeviceMemoryChunk::DeviceMemoryChunk()
{

}

DeviceMemoryChunk::~DeviceMemoryChunk()
{

}

void DeviceMemoryChunk::Allocate(uint32_t inElementCount, DeviceSize inSize, uint32_t inMemTypeBits, MemoryPropertyFlags inMemPropertyFlags)
{
	numSlots = inElementCount;
	elementSize = inSize;
	memory.Allocate(elementSize * numSlots, inMemTypeBits, inMemPropertyFlags);

	freeSlots.clear();
	for (uint32_t index = 0; index < numSlots; index++)
	{
		freeSlots.push_back(index);
	}
}

void DeviceMemoryChunk::Allocate(uint32_t inElementCount, const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags)
{
	Allocate(inElementCount, inMemRequirements.size, inMemRequirements.memoryTypeBits, inMemPropertyFlags);
}

void DeviceMemoryChunk::Free()
{
	memory.Free();
}

DeviceSize DeviceMemoryChunk::AcquireSlot()
{
	DeviceSize slot = freeSlots.front();
	freeSlots.pop_front();
	return slot;
}

void DeviceMemoryChunk::ReleaseSlot(DeviceSize inSlot)
{
	freeSlots.push_front(inSlot);
}

DeviceSize DeviceMemoryChunk::GetSlotOffset(DeviceSize inSlot)
{
	return elementSize * inSlot;
}

DeviceMemoryWrapper& DeviceMemoryChunk::GetMemory()
{
	return memory;
}

bool DeviceMemoryChunk::HasFreeSpace()
{
	return !freeSlots.empty();
}
