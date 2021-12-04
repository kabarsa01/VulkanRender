#include "ArrayMemoryChunk.h"

namespace CGE
{

	ArrayMemoryChunk::ArrayMemoryChunk(vk::DeviceSize segmentSize, vk::DeviceSize chunkSize, vk::MemoryRequirements requirements, vk::MemoryPropertyFlags flags)
		: IMemoryChunk(segmentSize, chunkSize)
	{
		m_memory.SetRequirements(requirements);
		m_memory.SetPropertyFlags(flags);
		m_memory.SetSize(GetChunkSize());
		m_memory.Allocate();

		m_freeSegmentBlocks.reserve(4 * 1024);
		m_freeSegmentBlocks.emplace_back(MemRecord{ 0, static_cast<uint32_t>(GetChunkSize() / GetSegmentSize()) });
	}

	ArrayMemoryChunk::~ArrayMemoryChunk()
	{
		m_memory.Free();
	}

	MemoryPosition ArrayMemoryChunk::AcquireSegment(vk::DeviceSize size)
	{
		if (m_freeSegmentBlocks.empty())
		{
			return {};
		}
		vk::DeviceSize segmentsNeeded = size / GetSegmentSize();
		if ((size % GetSegmentSize()) > 0)
		{
			++segmentsNeeded;
		}

		MemoryPosition result;
		uint32_t blockIndex = 0xffffffff;
		for (uint32_t idx = 0; idx < m_freeSegmentBlocks.size(); ++idx)
		{
			MemRecord& rec = m_freeSegmentBlocks[idx];
			if (rec.size < segmentsNeeded)
			{
				continue;
			}

			result.offset = rec.offset * GetSegmentSize();
			result.size = segmentsNeeded * GetSegmentSize();
			rec.offset += segmentsNeeded;
			rec.size -= segmentsNeeded;

			if (rec.size == 0)
			{
				blockIndex = idx;
			}

			break;
		}
		if (blockIndex != 0xffffffff)
		{
			m_freeSegmentBlocks.erase(m_freeSegmentBlocks.begin() + blockIndex);
		}

		result.valid = result.size > 0;
		result.memory = m_memory;

		return result;
	}

	void ArrayMemoryChunk::ReleaseSegment(const MemoryPosition& memoryPosition)
	{
		MemRecord rec;
		rec.offset = static_cast<uint32_t>(memoryPosition.offset / GetSegmentSize());
		rec.size = static_cast<uint32_t>(memoryPosition.size / GetSegmentSize());

		uint32_t index;
		std::vector<MemRecord>::iterator itemIter = std::lower_bound(m_freeSegmentBlocks.begin(), m_freeSegmentBlocks.end(), rec);
		if (itemIter == m_freeSegmentBlocks.end())
		{
			index = static_cast<uint32_t>(m_freeSegmentBlocks.size());
			m_freeSegmentBlocks.push_back(rec);
		}
		else
		{
			index = static_cast<uint32_t>(std::distance(m_freeSegmentBlocks.begin(), itemIter));
			m_freeSegmentBlocks.insert(itemIter, rec);
		}

		// merging order is from the end so that the index could stay relevant
		if (index < (m_freeSegmentBlocks.size() - 1))
		{
			MergeBlocks(index, index + 1);
		}
		if (index > 0)
		{
			MergeBlocks(index - 1, index);
		}
	}

	void ArrayMemoryChunk::MergeBlocks(uint32_t first, uint32_t second)
	{
		MemRecord& rec1 = m_freeSegmentBlocks[first];
		MemRecord& rec2 = m_freeSegmentBlocks[second];
		if ((rec1.offset + rec1.size) == rec2.offset)
		{
			rec1.size += rec2.size;
			m_freeSegmentBlocks.erase(m_freeSegmentBlocks.begin() + second);
		}
	}

}

