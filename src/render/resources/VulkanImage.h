#pragma once

#include "vulkan/vulkan.hpp"
#include "../objects/VulkanDevice.h"
#include "../memory/DeviceMemoryManager.h"
#include "VulkanBuffer.h"
#include <vector>

using namespace VULKAN_HPP_NAMESPACE;

// Wrapper for a Vulkan image resource
class VulkanImage
{
public:
	ImageCreateInfo createInfo;

	VulkanImage(bool inScoped = false);
	virtual ~VulkanImage();

	void Create(VulkanDevice* inDevice);
	void Destroy();

	void SetData(const std::vector<char>& inData);
	void SetData(DeviceSize inSize, char* inData);
	void BindMemory(MemoryPropertyFlags inMemoryPropertyFlags);

	void Transfer(CommandBuffer* inCmdBuffer, uint32_t inQueueFamilyIndex);
	BufferImageCopy CreateBufferImageCopy();
	void LayoutTransition(CommandBuffer* inCmdBuffer, ImageLayout inOldLayout, ImageLayout inNewLayout);

	ImageMemoryBarrier CreateBarrier(
		ImageLayout inOldLayout,
		ImageLayout inNewLayout,
		uint32_t inSrcQueue,
		uint32_t inDstQueue,
		AccessFlags inSrcAccessMask,
		AccessFlags inDstAccessMask,
		ImageAspectFlags inAspectFlags,
		uint32_t inBaseMipLevel,
		uint32_t inMipLevelCount,
		uint32_t inBaseArrayLayer,
		uint32_t inArrayLayerCount
	);
	ImageMemoryBarrier CreateBarrier(
		ImageLayout inOldLayout,
		ImageLayout inNewLayout,
		uint32_t inSrcQueue,
		uint32_t inDstQueue,
		AccessFlags inSrcAccessMask,
		AccessFlags inDstAccessMask,
		ImageSubresourceRange inSubresourceRange
	);
	ImageMemoryBarrier CreateLayoutBarrier(
		ImageLayout inOldLayout,
		ImageLayout inNewLayout,
		AccessFlags inSrcAccessMask,
		AccessFlags inDstAccessMask,
		ImageAspectFlags inAspectFlags,
		uint32_t inBaseMipLevel,
		uint32_t inMipLevelCount,
		uint32_t inBaseArrayLayer,
		uint32_t inArrayLayerCount
	);

	Image& GetImage();
	const Image& GetImage() const;
	MemoryRequirements GetMemoryRequirements();
	VulkanBuffer* CreateStagingBuffer(SharingMode inSharingMode, uint32_t inQueueFamilyIndex);

	operator Image() const { return image; }
	operator bool() const { return image; }
protected:
	VulkanDevice* vulkanDevice;
	Image image;
	MemoryRecord memoryRecord;
	VulkanBuffer stagingBuffer;
	std::vector<char> data;

	bool scoped;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
};


