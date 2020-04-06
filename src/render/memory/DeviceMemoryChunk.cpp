#include "DeviceMemoryChunk.h"

namespace
{
	static const unsigned char buddyFlag = 1 << 7;
	static const unsigned char freeFlag = 1;
	static const unsigned char splitFlag = 1 << 1;
};

DeviceMemoryChunk::DeviceMemoryChunk(DeviceSize inSegmentSize)
	: segmentSize(inSegmentSize)
	, memoryTree{}
{
	memoryTree[0] = freeFlag;
	for (uint64_t index = 1; index < MEM_TREE_SIZE; index++)
	{
		unsigned char value = freeFlag;
		value |= ( (index % 2) * buddyFlag );
		memoryTree[index] = value;
	}
}

DeviceMemoryChunk::~DeviceMemoryChunk()
{

}

void DeviceMemoryChunk::Allocate(uint32_t inMemTypeBits, MemoryPropertyFlags inMemPropertyFlags)
{
	DeviceSize chunkSize = segmentSize * MEM_SEGMENTS_COUNT;
	memory.Allocate(chunkSize, inMemTypeBits, inMemPropertyFlags);
}

void DeviceMemoryChunk::Allocate(const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags)
{
	Allocate(inMemRequirements.memoryTypeBits, inMemPropertyFlags);
}

void DeviceMemoryChunk::Free()
{
	memory.Free();
}

MemoryChunkPosition DeviceMemoryChunk::AcquireSegment(DeviceSize inSize)
{
	DeviceSize order = 0;
	DeviceSize requiredSize = segmentSize;
	uint32_t layer = 0;
	while (inSize > requiredSize)
	{
		requiredSize <<= 1;
		layer++;
	}

	uint32_t currentLayer = MEM_LAYERS_COUNT - 1;
	uint32_t nodeIndex = GetLayerStartIndex(currentLayer);
	uint32_t targetIndex;
	bool success = FindSegmentIndex(currentLayer, layer, nodeIndex, targetIndex);
	if (!success)
	{
		return {false};
	}
	MarkNotFreeUp(layer, targetIndex);

	MemoryChunkPosition pos;
	pos.valid = true;
	pos.index = targetIndex;
	pos.layer = layer;
	pos.memory = memory;
	pos.offset = CalculateOffset(layer, targetIndex);

	return pos;
}

DeviceSize DeviceMemoryChunk::AcquireSlot()
{
	DeviceSize slot = 0;// = freeSlots.front();
	return slot;
}

void DeviceMemoryChunk::ReleaseSlot(DeviceSize inSlot)
{
}

DeviceSize DeviceMemoryChunk::GetSlotOffset(DeviceSize inSlot)
{
	return segmentSize * inSlot;
}

DeviceMemoryWrapper& DeviceMemoryChunk::GetMemory()
{
	return memory;
}

bool DeviceMemoryChunk::HasFreeSpace()
{
	return true;
}

uint32_t DeviceMemoryChunk::GetLayerStartIndex(uint32_t inLayer)
{
	uint32_t layerInverted = MEM_LAYERS_COUNT - inLayer - 1;
	uint32_t layerStartIndex = (1 << layerInverted) - 1;
	return layerStartIndex;
}

uint32_t DeviceMemoryChunk::GetParentIndex(uint32_t inIndex)
{
	uint32_t parentIndex = ((inIndex + 1) >> 1) - 1;
	return parentIndex;
}

uint32_t DeviceMemoryChunk::GetChildIndex(uint32_t inIndex)
{
	uint32_t childIndex = ((inIndex + 1) << 1) - 1;
	return childIndex;
}

uint32_t DeviceMemoryChunk::GetSiblingIndex(uint32_t inIndex)
{
	unsigned char value = memoryTree[inIndex];
	int32_t offset = (value & buddyFlag) ? 1 : -1;
	uint32_t buddyIndex = inIndex + offset;
	return buddyIndex;
}

DeviceSize DeviceMemoryChunk::CalculateOffset(uint32_t inLayer, uint32_t inIndex)
{
	uint32_t baseLayerIndex = GetLayerStartIndex(inLayer);
	DeviceSize layerSegmentSize = segmentSize * (uint64_t(1) << inLayer);
	return (inIndex - baseLayerIndex) * layerSegmentSize;
}

bool DeviceMemoryChunk::FindSegmentIndex(uint32_t inCurrentLayer, uint32_t inTargetLayer, uint32_t inIndex, uint32_t& outTargetIndex)
{
	const unsigned char value = memoryTree[inIndex];

	bool isFree = value & freeFlag;
	if (!isFree)
	{
		return false;
	}

	bool isTargetLayer = inCurrentLayer == inTargetLayer;
	if (!isTargetLayer)
	{
		memoryTree[inIndex] |= splitFlag;
		uint32_t childBaseIndex = GetChildIndex(inIndex);
		if (FindSegmentIndex(inCurrentLayer + 1, inTargetLayer, childBaseIndex, outTargetIndex))
		{
			return true;
		}
		return FindSegmentIndex(inCurrentLayer + 1, inTargetLayer, childBaseIndex + 1, outTargetIndex);
	}

	bool isSplit = value & splitFlag;
	if (isSplit)
	{
		return false;
	}

	memoryTree[inIndex] &= (~freeFlag);
	outTargetIndex = inIndex;

	return true;
}

void DeviceMemoryChunk::MarkFreeUp(uint32_t inStartLayer, uint32_t inIndex)
{
	if (inStartLayer >= MEM_LAYERS_COUNT)
	{
		return;
	}

	uint32_t siblingIndex = GetSiblingIndex(inIndex);
	uint32_t parentIndex = GetParentIndex(inIndex);

	memoryTree[inIndex] |= freeFlag;
	bool isSiblingSplit = memoryTree[siblingIndex] & splitFlag;
	if (!isSiblingSplit)
	{
		memoryTree[parentIndex] &= (~splitFlag);
	}

	memoryTree[parentIndex] |= freeFlag;
	MarkFreeUp(inStartLayer + 1, parentIndex);
}

void DeviceMemoryChunk::MarkNotFreeUp(uint32_t inStartLayer, uint32_t inIndex)
{
	if (inStartLayer >= MEM_LAYERS_COUNT)
	{
		return;
	}

	bool isFree = memoryTree[inIndex] & freeFlag;
	bool isSiblingFree = memoryTree[GetSiblingIndex(inIndex)] & freeFlag;
	if ((!isFree) && (!isSiblingFree))
	{
		uint32_t parentIndex = GetParentIndex(inIndex);
		memoryTree[parentIndex] &= (~freeFlag);
		MarkNotFreeUp(inStartLayer + 1, parentIndex);
	}
}
