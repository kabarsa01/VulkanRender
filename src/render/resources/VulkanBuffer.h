#pragma once

#include "vulkan/vulkan.hpp"
#include "../memory/DeviceMemoryManager.h"
#include "../objects/VulkanDevice.h"

using namespace VULKAN_HPP_NAMESPACE;

class VulkanBuffer
{
public:
	BufferCreateInfo createInfo;

	VulkanBuffer(bool inScoped = false);
	virtual ~VulkanBuffer();

	void Create(VulkanDevice* inDevice);
	void Destroy();
	void BindMemory(MemoryPropertyFlags inMemPropertyFlags);
	void BindMemory(const DeviceMemory& inDeviceMemory, DeviceSize inMemOffset);

	Buffer& GetBuffer();
	Buffer GetBuffer() const;
	MemoryRequirements GetMemoryRequirements();
	MemoryRecord& GetMemoryRecord();

	operator Buffer() const { return buffer; }
	operator bool() const { return buffer; }

	static void SubmitCopyCommand(const VulkanBuffer& inSrc, const VulkanBuffer& inDst);
protected:
	bool scoped;
	VulkanDevice* vulkanDevice;
	Buffer buffer;
	MemoryRecord memRecord;
};
