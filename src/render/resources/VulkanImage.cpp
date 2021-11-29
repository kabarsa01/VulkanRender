#include "VulkanImage.h"
#include "core/Engine.h"
#include "../Renderer.h"

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
		, m_cleanup(false)
		, m_image(nullptr)
	{

	}

	VulkanImage::VulkanImage(bool inScoped)
		: m_vulkanDevice(nullptr)
		, m_scoped(inScoped)
		, m_cleanup(false)
		, m_image(nullptr)
	{
	
	}
	
	VulkanImage& VulkanImage::operator=(const VulkanImage& otherImage)
	{
		m_vulkanDevice = otherImage.m_vulkanDevice;
		m_image = otherImage.m_image;
		m_memoryRecord = otherImage.m_memoryRecord;
		m_requirements = otherImage.m_requirements;

		createInfo = otherImage.createInfo;

		m_scoped = otherImage.m_scoped;
		m_cleanup = otherImage.m_cleanup;
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
	
	void VulkanImage::Create()
	{
		if (m_image)
		{
			return;
		}
		m_vulkanDevice = &Engine::GetRendererInstance()->GetVulkanDevice();
	
		m_width = createInfo.extent.width;
		m_height = createInfo.extent.height;
		m_depth = createInfo.extent.depth;
		m_mips = std::min(createInfo.mipLevels, uint32_t(floor(log2(std::max(std::max(m_width, m_height), m_depth))) + 1));
		createInfo.mipLevels = m_mips;
	
		m_image = m_vulkanDevice->GetDevice().createImage(createInfo);
		m_requirements = m_vulkanDevice->GetDevice().getImageMemoryRequirements(m_image);

		BindMemory(vk::MemoryPropertyFlagBits::eDeviceLocal);
	}
	
	void VulkanImage::CreateFromExternal(vk::Image image, bool cleanup)
	{
		m_vulkanDevice = &Engine::GetRendererInstance()->GetVulkanDevice();
		m_cleanup = cleanup;
		m_image = image;
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
		if (m_cleanup && m_image)
		{
			m_vulkanDevice->GetDevice().destroyImage(m_image);
			m_image = nullptr;
			DeviceMemoryManager::GetInstance()->ReturnMemory(m_memoryRecord);
		}
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
	
}
