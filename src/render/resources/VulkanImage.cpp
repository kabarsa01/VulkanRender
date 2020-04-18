#include "VulkanImage.h"
#include "core/Engine.h"


VulkanImage::VulkanImage(bool inScoped)
	: scoped(inScoped)
{

}

VulkanImage::~VulkanImage()
{
	if (scoped)
	{
		Destroy();
	}
}

void VulkanImage::Create(VulkanDevice* inDevice)
{
	if (image || !inDevice)
	{
		return;
	}
	vulkanDevice = inDevice;
	image = vulkanDevice->GetDevice().createImage(createInfo);
}

void VulkanImage::Destroy()
{
	if (stagingBuffer)
	{
		stagingBuffer.Destroy();
	}
	if (image && vulkanDevice)
	{
		vulkanDevice->GetDevice().destroyImage(image);
		image = nullptr;
		DeviceMemoryManager::GetInstance()->ReturnMemory(memoryRecord);
	}

}

void VulkanImage::SetData(const std::vector<char>& inData)
{
	data = inData;
}

void VulkanImage::SetData(DeviceSize inSize, char* inData)
{
	data.assign(inData, inData + inSize);
}

void VulkanImage::BindMemory(MemoryPropertyFlags inMemoryPropertyFlags)
{
	DeviceMemoryManager* dmm = DeviceMemoryManager::GetInstance();
	memoryRecord = dmm->RequestMemory(GetMemoryRequirements(), inMemoryPropertyFlags);
	vulkanDevice->GetDevice().bindImageMemory(image, memoryRecord.pos.memory, memoryRecord.pos.offset);
}

void VulkanImage::Transfer(CommandBuffer* inCmdBuffer, uint32_t inQueueFamilyIndex)
{
	if (!stagingBuffer)
	{
		CreateStagingBuffer(SharingMode::eExclusive, inQueueFamilyIndex);
	}

	std::array<BufferImageCopy, 1> copyArray = { CreateBufferImageCopy() };
	inCmdBuffer->copyBufferToImage(stagingBuffer, image, ImageLayout::eTransferDstOptimal, 1, copyArray.data());
}

BufferImageCopy VulkanImage::CreateBufferImageCopy()
{
	BufferImageCopy imageCopy;
	imageCopy.setBufferOffset(0);
	imageCopy.setBufferImageHeight(0);
	imageCopy.setBufferRowLength(0);
	imageCopy.setImageExtent(Extent3D(width, height, depth));
	imageCopy.setImageOffset(Offset3D(0, 0, 0));
	imageCopy.imageSubresource.setAspectMask(ImageAspectFlagBits::eColor);
	imageCopy.imageSubresource.setBaseArrayLayer(0);
	imageCopy.imageSubresource.setLayerCount(1);
	imageCopy.imageSubresource.setMipLevel(0);

	return imageCopy;
}

void VulkanImage::LayoutTransition(CommandBuffer* inCmdBuffer, ImageLayout inOldLayout, ImageLayout inNewLayout)
{
	ImageMemoryBarrier barrier = CreateLayoutBarrier(
		inOldLayout,
		inNewLayout,
		AccessFlagBits::eColorAttachmentWrite,
		AccessFlagBits::eShaderRead,
		ImageAspectFlagBits::eColor,
		0, 1, 0, 1
	);

	inCmdBuffer->pipelineBarrier(PipelineStageFlagBits::eColorAttachmentOutput, PipelineStageFlagBits::eFragmentShader, DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);
}

ImageMemoryBarrier VulkanImage::CreateBarrier(
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
	uint32_t inArrayLayerCount)
{
	ImageMemoryBarrier barrier;
	barrier.setImage(image);
	barrier.setOldLayout(inOldLayout);
	barrier.setNewLayout(inNewLayout);
	barrier.setSrcQueueFamilyIndex(inSrcQueue);
	barrier.setDstQueueFamilyIndex(inDstQueue);
	barrier.subresourceRange.setAspectMask(inAspectFlags);
	barrier.subresourceRange.setBaseMipLevel(inBaseMipLevel);
	barrier.subresourceRange.setLevelCount(inMipLevelCount);
	barrier.subresourceRange.setBaseArrayLayer(inBaseArrayLayer);
	barrier.subresourceRange.setLayerCount(inArrayLayerCount);
	barrier.setSrcAccessMask(inSrcAccessMask);
	barrier.setDstAccessMask(inDstAccessMask);

	return barrier;
}

ImageMemoryBarrier VulkanImage::CreateBarrier(
	ImageLayout inOldLayout, 
	ImageLayout inNewLayout, 
	uint32_t inSrcQueue, 
	uint32_t inDstQueue, 
	AccessFlags inSrcAccessMask, 
	AccessFlags inDstAccessMask, 
	ImageSubresourceRange inSubresourceRange)
{
	ImageMemoryBarrier barrier;
	barrier.setImage(image);
	barrier.setOldLayout(inOldLayout);
	barrier.setNewLayout(inNewLayout);
	barrier.setSrcQueueFamilyIndex(inSrcQueue);
	barrier.setDstQueueFamilyIndex(inDstQueue);
	barrier.setSubresourceRange(inSubresourceRange);
	barrier.setSrcAccessMask(inSrcAccessMask);
	barrier.setDstAccessMask(inDstAccessMask);

	return barrier;
}

ImageMemoryBarrier VulkanImage::CreateLayoutBarrier(ImageLayout inOldLayout, ImageLayout inNewLayout, AccessFlags inSrcAccessMask, AccessFlags inDstAccessMask, ImageAspectFlags inAspectFlags, uint32_t inBaseMipLevel, uint32_t inMipLevelCount, uint32_t inBaseArrayLayer, uint32_t inArrayLayerCount)
{
	return CreateBarrier(
		inOldLayout,
		inNewLayout,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		inSrcAccessMask,
		inDstAccessMask,
		inAspectFlags,
		inBaseMipLevel,
		inMipLevelCount,
		inBaseArrayLayer,
		inArrayLayerCount
	);
}

Image& VulkanImage::GetImage()
{
	return image;
}

const Image& VulkanImage::GetImage() const
{
	return image;
}

MemoryRequirements VulkanImage::GetMemoryRequirements()
{
	return vulkanDevice->GetDevice().getImageMemoryRequirements(image);
}

VulkanBuffer* VulkanImage::CreateStagingBuffer(SharingMode inSharingMode, uint32_t inQueueFamilyIndex)
{
	if (stagingBuffer)
	{
		return &stagingBuffer;
	}

	DeviceSize size = width * height * depth * 4;

	stagingBuffer.createInfo.setSize(size);
	stagingBuffer.createInfo.setSharingMode(inSharingMode);
	stagingBuffer.createInfo.setUsage(BufferUsageFlagBits::eTransferSrc);
	stagingBuffer.createInfo.setQueueFamilyIndexCount(1);
	stagingBuffer.createInfo.setPQueueFamilyIndices(&inQueueFamilyIndex);
	stagingBuffer.Create(vulkanDevice);
	stagingBuffer.BindMemory(MemoryPropertyFlagBits::eHostVisible);
	MemoryRecord& rec = stagingBuffer.GetMemoryRecord();
	rec.pos.memory.MapCopyUnmap(MemoryMapFlags(), rec.pos.offset, size, data.data(), 0, size);

	return &stagingBuffer;
}

