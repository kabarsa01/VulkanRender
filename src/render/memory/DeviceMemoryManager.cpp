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

DeviceSize DeviceMemoryManager::GetRangeBase(uint32_t inIndex)
{
	return baseMemorySegmentSize << (inIndex * (sizeRangeShift + 1));
}

DeviceSize DeviceMemoryManager::GetRangeMax(uint32_t inIndex)
{
	return GetRangeBase(inIndex) << sizeRangeShift;
}

uint32_t DeviceMemoryManager::GetRangeIndex(DeviceSize inSize)
{
	for (uint32_t index = 0; index < maxRanges; index++)
	{
		DeviceSize baseRangeSize = baseMemorySegmentSize << ( index * (sizeRangeShift + 1) );
		DeviceSize rangeUpperLimit = baseRangeSize << sizeRangeShift;
		if (inSize < rangeUpperLimit)
		{
			return index;
		}
	}

	return maxRanges - 1;
}

DeviceMemoryManager* DeviceMemoryManager::GetInstance()
{
	return &staticInstance;
}

MemoryRecord DeviceMemoryManager::RequestMemory(const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags)
{
	auto startTime = std::chrono::high_resolution_clock::now();

	MemoryRecord memoryRecord;

	uint64_t memTypeIndex = DeviceMemoryWrapper::FindMemoryTypeStatic(inMemRequirements.memoryTypeBits, inMemPropertyFlags);
	DeviceSize requiredSize = inMemRequirements.size;
	uint64_t rangeIndex = GetRangeIndex(requiredSize);
	uint64_t regionHash = rangeIndex | (memTypeIndex << 32);

	std::vector<DeviceMemoryChunk>& chunkArray = memRegions[regionHash];
	for (uint64_t index = 0; index < chunkArray.size(); index++)
	{
		DeviceMemoryChunk& chunk = chunkArray[index];
		if (chunk.HasFreeSpace())
		{
			MemoryPosition pos = chunk.AcquireSegment(requiredSize);
			if (pos.valid)
			{
				memoryRecord.regionHash = regionHash;
				memoryRecord.chunkIndex = index;
				memoryRecord.pos = pos;

				auto currentTime = std::chrono::high_resolution_clock::now();
				double deltaTime = std::chrono::duration<double, std::chrono::microseconds::period>(currentTime - startTime).count();

				printf("my allocation %f microseconds\n", deltaTime);

				return memoryRecord;
			}
		}
	}

	chunkArray.push_back(DeviceMemoryChunk(GetRangeBase(static_cast<uint32_t>(rangeIndex)), memoryTreeDepth));

	startTime = std::chrono::high_resolution_clock::now();

	DeviceMemoryChunk& chunk = chunkArray.back();
	chunk.Allocate(inMemRequirements, inMemPropertyFlags);

	auto currentTime = std::chrono::high_resolution_clock::now();
	double deltaTime = std::chrono::duration<double, std::chrono::microseconds::period>(currentTime - startTime).count();

	printf("vulkan allocation %f microseconds\n", deltaTime);

	MemoryPosition pos = chunk.AcquireSegment(requiredSize);

	memoryRecord.regionHash = regionHash;
	memoryRecord.chunkIndex = chunkArray.size() - 1;
	memoryRecord.pos = pos;

	currentTime = std::chrono::high_resolution_clock::now();
	deltaTime = std::chrono::duration<double, std::chrono::microseconds::period>(currentTime - startTime).count();

	return memoryRecord;
}

void DeviceMemoryManager::ReturnMemory(const MemoryRecord& inMemoryPosition)
{
	memRegions[inMemoryPosition.regionHash][inMemoryPosition.chunkIndex].ReleaseSegment(inMemoryPosition.pos);
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




