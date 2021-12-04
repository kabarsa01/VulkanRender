#ifndef __I_MEMORY_CHUNK_H__
#define __I_MEMORY_CHUNK_H__

#include "vulkan/vulkan.hpp"
#include "../resources/VulkanDeviceMemory.h"

namespace CGE
{

	struct MemoryPosition
	{
		bool valid = false;
		uint32_t layer;
		uint32_t index;
		vk::DeviceSize offset;
		vk::DeviceSize size;
		VulkanDeviceMemory memory;

		MemoryPosition()
			: valid(false)
			, layer(-1)
			, index(-1)
			, offset(-1)
			, size(0)
			, memory()
		{}
	};

	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------

	// we'll try to do it the RAII way
	class IMemoryChunk
	{
	public:
		IMemoryChunk(vk::DeviceSize segmentSize, vk::DeviceSize chunkSize);
		virtual ~IMemoryChunk();

		vk::DeviceSize GetSegmentSize() { return m_segmentSize; }
		vk::DeviceSize GetChunkSize() { return m_chunkSize; }

		virtual MemoryPosition AcquireSegment(vk::DeviceSize size) = 0;
		virtual void ReleaseSegment(const MemoryPosition& memoryPosition) = 0;
		virtual bool HasFreeSpace() = 0;
	private:
		vk::DeviceSize m_segmentSize;
		vk::DeviceSize m_chunkSize;
	};

}

#endif

