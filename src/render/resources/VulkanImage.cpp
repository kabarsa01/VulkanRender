#include "VulkanImage.h"
#include "core/Engine.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::ImageViewCreateInfo;
	using VULKAN_HPP_NAMESPACE::ComponentMapping;
	using VULKAN_HPP_NAMESPACE::Extent3D;
	using VULKAN_HPP_NAMESPACE::Offset3D;
	using VULKAN_HPP_NAMESPACE::ImageAspectFlagBits;
	using VULKAN_HPP_NAMESPACE::AccessFlagBits;
	using VULKAN_HPP_NAMESPACE::PipelineStageFlagBits;
	using VULKAN_HPP_NAMESPACE::DependencyFlags;
	using VULKAN_HPP_NAMESPACE::BufferUsageFlagBits;

	VulkanImage::VulkanImage()
		: m_vulkanDevice(nullptr)
		, m_scoped(false)
		, m_image(nullptr)
	{

	}

	VulkanImage::VulkanImage(bool inScoped)
		: m_vulkanDevice(nullptr)
		, m_scoped(inScoped)
		, m_image(nullptr)
	{
	
	}
	
	VulkanImage& VulkanImage::operator=(const VulkanImage& otherImage)
	{
		m_vulkanDevice = otherImage.m_vulkanDevice;
		m_image = otherImage.m_image;
		m_memoryRecord = otherImage.m_memoryRecord;
		m_requirements = otherImage.m_requirements;
		m_stagingBuffer = otherImage.m_stagingBuffer;
		m_data = otherImage.m_data;

		createInfo = otherImage.createInfo;

		m_scoped = otherImage.m_scoped;
		m_width = otherImage.m_width;
		m_height = otherImage.m_height;
		m_depth = otherImage.m_depth;
		m_mips = otherImage.m_mips;

		return *this;
	}

	VulkanImage::~VulkanImage()
	{
		if (m_scoped)
		{
			Destroy();
		}
	}
	
	void VulkanImage::Create(VulkanDevice* inDevice)
	{
		if (m_image || !inDevice)
		{
			return;
		}
		m_vulkanDevice = inDevice;
	
		m_width = createInfo.extent.width;
		m_height = createInfo.extent.height;
		m_depth = createInfo.extent.depth;
		m_mips = std::min(createInfo.mipLevels, uint32_t(floor(log2(std::max(std::max(m_width, m_height), m_depth))) + 1));
		createInfo.mipLevels = m_mips;
	
		m_image = m_vulkanDevice->GetDevice().createImage(createInfo);
		m_requirements = m_vulkanDevice->GetDevice().getImageMemoryRequirements(m_image);
	}
	
	ImageView VulkanImage::CreateView(ImageSubresourceRange inSubRange, ImageViewType inViewType) const
	{
		ImageViewCreateInfo imageViewInfo;
		imageViewInfo.setComponents(ComponentMapping());
		imageViewInfo.setFormat(createInfo.format);
		imageViewInfo.setImage(m_image);
		imageViewInfo.setSubresourceRange(inSubRange);
		imageViewInfo.setViewType(inViewType);
	
		return m_vulkanDevice->GetDevice().createImageView(imageViewInfo);
	}

	void VulkanImage::Destroy()
	{
		DestroyStagingBuffer();
		if (m_image)
		{
			m_vulkanDevice->GetDevice().destroyImage(m_image);
			m_image = nullptr;
			DeviceMemoryManager::GetInstance()->ReturnMemory(m_memoryRecord);
		}
	}
	
	void VulkanImage::SetData(const std::vector<char>& inData)
	{
		m_data = inData;
	}
	
	void VulkanImage::SetData(DeviceSize inSize, char* inData)
	{
		m_data.assign(inData, inData + inSize);
	}
	
	void VulkanImage::BindMemory(MemoryPropertyFlags inMemoryPropertyFlags)
	{
		if (m_memoryRecord.pos.valid)
		{
			return;
		}
		DeviceMemoryManager* dmm = DeviceMemoryManager::GetInstance();
		m_memoryRecord = dmm->RequestMemory(GetMemoryRequirements(), inMemoryPropertyFlags);
		m_vulkanDevice->GetDevice().bindImageMemory(m_image, m_memoryRecord.pos.memory, m_memoryRecord.pos.offset);
	}
	
	void VulkanImage::Transfer(CommandBuffer* inCmdBuffer, uint32_t inQueueFamilyIndex)
	{
		if (!m_stagingBuffer)
		{
			CreateStagingBuffer(SharingMode::eExclusive, inQueueFamilyIndex);
		}
	
		std::array<BufferImageCopy, 1> copyArray = { CreateBufferImageCopy() };
		inCmdBuffer->copyBufferToImage(m_stagingBuffer, m_image, ImageLayout::eTransferDstOptimal, 1, copyArray.data());
	}
	
	BufferImageCopy VulkanImage::CreateBufferImageCopy()
	{
		BufferImageCopy imageCopy;
		imageCopy.setBufferOffset(0);
		imageCopy.setBufferImageHeight(0);
		imageCopy.setBufferRowLength(0);
		imageCopy.setImageExtent(Extent3D(m_width, m_height, m_depth));
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
		uint32_t inArrayLayerCount) const
	{
		ImageMemoryBarrier barrier;
		barrier.setImage(m_image);
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
		ImageSubresourceRange inSubresourceRange) const
	{
		ImageMemoryBarrier barrier;
		barrier.setImage(m_image);
		barrier.setOldLayout(inOldLayout);
		barrier.setNewLayout(inNewLayout);
		barrier.setSrcQueueFamilyIndex(inSrcQueue);
		barrier.setDstQueueFamilyIndex(inDstQueue);
		barrier.setSubresourceRange(inSubresourceRange);
		barrier.setSrcAccessMask(inSrcAccessMask);
		barrier.setDstAccessMask(inDstAccessMask);
	
		return barrier;
	}
	
	ImageMemoryBarrier VulkanImage::CreateLayoutBarrier(
		ImageLayout inOldLayout, 
		ImageLayout inNewLayout, 
		AccessFlags inSrcAccessMask, 
		AccessFlags inDstAccessMask, 
		ImageAspectFlags inAspectFlags, 
		uint32_t inBaseMipLevel, 
		uint32_t inMipLevelCount, 
		uint32_t inBaseArrayLayer, 
		uint32_t inArrayLayerCount) const
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
		return m_image;
	}
	
	const Image& VulkanImage::GetImage() const
	{
		return m_image;
	}
	
	MemoryRequirements VulkanImage::GetMemoryRequirements()
	{
		return m_requirements;
	}
	
	VulkanBuffer* VulkanImage::CreateStagingBuffer(char* inData)
	{
		return CreateStagingBuffer(SharingMode::eExclusive, 0, inData);
	}
	
	VulkanBuffer* VulkanImage::CreateStagingBuffer(SharingMode inSharingMode, uint32_t inQueueFamilyIndex)
	{
		return CreateStagingBuffer(inSharingMode, inQueueFamilyIndex, m_data.data());
	}
	
	VulkanBuffer* VulkanImage::CreateStagingBuffer(SharingMode inSharingMode, uint32_t inQueueFamilyIndex, char* inData)
	{
		if (m_stagingBuffer)
		{
			return &m_stagingBuffer;
		}
	
		DeviceSize size = m_width * m_height * m_depth * 4;//memoryRequirements.size;//
	
		m_stagingBuffer.createInfo.setSize(size);
		m_stagingBuffer.createInfo.setSharingMode(inSharingMode);
		m_stagingBuffer.createInfo.setUsage(BufferUsageFlagBits::eTransferSrc);
		m_stagingBuffer.createInfo.setQueueFamilyIndexCount(1);
		m_stagingBuffer.createInfo.setPQueueFamilyIndices(&inQueueFamilyIndex);
		m_stagingBuffer.Create(false);
//		m_stagingBuffer.BindMemory(MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent);
		MemoryRecord& rec = m_stagingBuffer.GetMemoryRecord();
		rec.pos.memory.MapCopyUnmap(MemoryMapFlags(), rec.pos.offset, size, inData, 0, size);
	
		return &m_stagingBuffer;
	}
	
	void VulkanImage::DestroyStagingBuffer()
	{
		if (m_stagingBuffer)
		{
			m_stagingBuffer.Destroy();
		}
	}
	
}
