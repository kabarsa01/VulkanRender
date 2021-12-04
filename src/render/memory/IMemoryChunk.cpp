#include "IMemoryChunk.h"

namespace CGE
{

	IMemoryChunk::IMemoryChunk(vk::DeviceSize segmentSize, vk::DeviceSize chunkSize)
		: m_segmentSize(segmentSize)
		, m_chunkSize(chunkSize)
	{
		m_chunkSize = m_segmentSize * (m_chunkSize / m_segmentSize);
	}

	IMemoryChunk::~IMemoryChunk()
	{

	}

}

