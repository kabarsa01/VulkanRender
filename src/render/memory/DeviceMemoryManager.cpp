#include "DeviceMemoryManager.h"

DeviceMemoryManager DeviceMemoryManager::staticInstance;

namespace
{
	static const uint32_t CHUNK_ELEMENT_COUNT = 64;
};

DeviceMemoryManager::DeviceMemoryManager()
{

}

DeviceMemoryManager::~DeviceMemoryManager()
{

}

DeviceMemoryManager* DeviceMemoryManager::GetInstance()
{
	return &staticInstance;
}

MemoryRecord DeviceMemoryManager::RequestMemory(const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags)
{
	MemoryRecord memoryRecord;

	DeviceSize memTypeIndex = DeviceMemoryWrapper::FindMemoryType(inMemRequirements.memoryTypeBits, inMemPropertyFlags);
	DeviceSize elementSize = inMemRequirements.size;
	uint64_t regionHash = elementSize & (memTypeIndex << 32);

	std::vector<DeviceMemoryChunk>& chunkArray = memRegions[regionHash];
	for (uint64_t index = 0; index < chunkArray.size(); index++)
	{
		DeviceMemoryChunk& chunk = chunkArray[index];
		if (chunk.HasFreeSpace())
		{
			memoryRecord.regionHash = regionHash;
			memoryRecord.chunkIndex = index;
			memoryRecord.chunkSlot = chunk.AcquireSlot();
			memoryRecord.memoryOffset = chunk.GetSlotOffset(memoryRecord.chunkSlot);
			memoryRecord.memory = chunk.GetMemory();
			return memoryRecord;
		}
	}

	chunkArray.push_back(DeviceMemoryChunk());
	DeviceMemoryChunk& chunk = chunkArray.back();
	chunk.Allocate(CHUNK_ELEMENT_COUNT, inMemRequirements, inMemPropertyFlags);

	memoryRecord.regionHash = regionHash;
	memoryRecord.chunkIndex = chunkArray.size() - 1;
	memoryRecord.chunkSlot = chunk.AcquireSlot();
	memoryRecord.memoryOffset = chunk.GetSlotOffset(memoryRecord.chunkSlot);
	memoryRecord.memory = chunk.GetMemory();
	return memoryRecord;
}

void DeviceMemoryManager::ReturnMemory(const MemoryRecord& inMemoryPosition)
{
	memRegions[inMemoryPosition.regionHash][inMemoryPosition.chunkIndex].ReleaseSlot(inMemoryPosition.chunkSlot);
}

void DeviceMemoryManager::CleanupMemory()
{
	std::map<uint64_t, std::vector<DeviceMemoryChunk>>::iterator regionIter;
	for (regionIter = memRegions.begin(); regionIter != memRegions.end(); regionIter++)
	{
		std::vector<DeviceMemoryChunk>& chunks = regionIter->second;
		for (uint64_t index = 0; index < chunks.size(); index++)
		{
			chunks[index].Free();
		}
	}
}

DeviceMemoryChunk& DeviceMemoryManager::GetMemoryChunk(MemoryRecord inMemPosition)
{
	return memRegions[inMemPosition.regionHash][inMemPosition.chunkIndex];
}




