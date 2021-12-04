#ifndef __ARRAY_MEMORY_CHUNK_H__
#define __ARRAY_MEMORY_CHUNK_H__

#include "IMemoryChunk.h"
#include "vulkan/vulkan.hpp"

namespace CGE
{

	// offset and size is 4 bytes because it's counted in mem segments
	struct MemRecord
	{
		uint32_t offset;
		uint32_t size;
	};
	inline bool operator<(const MemRecord& rec1, const MemRecord& rec2) { return rec1.offset < rec2.offset; }

	//------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------

	class ArrayMemoryChunk : public IMemoryChunk
	{
	public:
		ArrayMemoryChunk(vk::DeviceSize segmentSize, vk::DeviceSize chunkSize, vk::MemoryRequirements requirements, vk::MemoryPropertyFlags flags);
		virtual ~ArrayMemoryChunk();

		MemoryPosition AcquireSegment(DeviceSize size) override;
		void ReleaseSegment(const MemoryPosition& memoryPosition) override;
		bool HasFreeSpace() override { return !m_freeSegmentBlocks.empty(); }
	private:
		VulkanDeviceMemory m_memory;
		std::vector<MemRecord> m_freeSegmentBlocks;

		void MergeBlocks(uint32_t first, uint32_t second);
	};

}

#endif
