#pragma once

#include "vulkan/vulkan.hpp"
#include "../memory/DeviceMemoryManager.h"
#include "../objects/VulkanDevice.h"
#include <vector>

using namespace VULKAN_HPP_NAMESPACE;

class VulkanBuffer
{
public:
	BufferCreateInfo createInfo;

	VulkanBuffer(bool inScoped = false);
	virtual ~VulkanBuffer();

	void Create(VulkanDevice* inDevice);
	void Destroy();

	void SetData(const std::vector<char>& inData);
	void SetData(DeviceSize inSize, char* inData);
	void CopyTo(DeviceSize inSize, char* inData);
	void CopyToBuffer(DeviceSize inSize, char* inData);
	void CopyToStagingBuffer(DeviceSize inSize, char* inData);
	void BindMemory(MemoryPropertyFlags inMemPropertyFlags);
	void BindMemory(const DeviceMemory& inDeviceMemory, DeviceSize inMemOffset);

	VulkanBuffer* CreateStagingBuffer();
	VulkanBuffer* CreateStagingBuffer(DeviceSize inSize, char* inData);
	BufferCopy CreateBufferCopy();
	BufferMemoryBarrier CreateMemoryBarrier(uint32_t inSrcQueue, uint32_t inDstQueue, AccessFlags inSrcAccessMask, AccessFlags inDstAccessMask);

	Buffer& GetBuffer();
	Buffer GetBuffer() const;
	MemoryRequirements GetMemoryRequirements();
	MemoryRecord& GetMemoryRecord();

	operator Buffer() const { return buffer; }
	operator bool() const { return buffer; }

	//static void SubmitCopyCommand(const VulkanBuffer& inSrc, const VulkanBuffer& inDst);
protected:
	VulkanDevice* vulkanDevice;
	Buffer buffer;
	MemoryRecord memRecord;
	std::vector<char> data;

	bool scoped = false;

	VulkanBuffer* stagingBuffer;
};
