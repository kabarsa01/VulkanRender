#pragma once

#include "vulkan/vulkan.hpp"
#include "../objects/VulkanDevice.h"
#include "../memory/DeviceMemoryManager.h"
#include <vector>

// Wrapper for a Vulkan image resource
namespace CGE
{
	using VULKAN_HPP_NAMESPACE::MemoryRequirements;
	using VULKAN_HPP_NAMESPACE::Image;
	using VULKAN_HPP_NAMESPACE::ImageView;
	using VULKAN_HPP_NAMESPACE::ImageCreateInfo;
	using VULKAN_HPP_NAMESPACE::ImageSubresourceRange;
	using VULKAN_HPP_NAMESPACE::ImageViewType;
	using VULKAN_HPP_NAMESPACE::DeviceSize;
	using VULKAN_HPP_NAMESPACE::MemoryPropertyFlags;
	using VULKAN_HPP_NAMESPACE::CommandBuffer;
	using VULKAN_HPP_NAMESPACE::BufferImageCopy;
	using VULKAN_HPP_NAMESPACE::ImageLayout;
	using VULKAN_HPP_NAMESPACE::ImageMemoryBarrier;
	using VULKAN_HPP_NAMESPACE::AccessFlags;
	using VULKAN_HPP_NAMESPACE::SharingMode;
	using VULKAN_HPP_NAMESPACE::ImageAspectFlags;

	class VulkanImage
	{
	public:
		ImageCreateInfo createInfo;
	
		VulkanImage();
		VulkanImage(bool inScoped);
		VulkanImage& operator=(const VulkanImage& otherImage);
		virtual ~VulkanImage();
	
		void Create();
		void CreateFromExternal(vk::Image image, bool cleanup);
		ImageView CreateView(ImageSubresourceRange inSubRange, ImageViewType inViewType) const;
		void Destroy();
	
		inline uint32_t GetWidth() { return m_width; }
		inline uint32_t GetHeight() { return m_height; }
		inline uint32_t GetDepth() { return m_depth; }
		inline uint32_t GetMips() { return m_mips; }

		BufferImageCopy CreateBufferImageCopy();
	
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
		) const;
		ImageMemoryBarrier CreateBarrier(
			ImageLayout inOldLayout,
			ImageLayout inNewLayout,
			uint32_t inSrcQueue,
			uint32_t inDstQueue,
			AccessFlags inSrcAccessMask,
			AccessFlags inDstAccessMask,
			ImageSubresourceRange inSubresourceRange
		) const;
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
		) const;
	
		Image& GetImage();
		const Image& GetImage() const;
		MemoryRequirements GetMemoryRequirements();
	
		operator Image() const { return m_image; }
		operator bool() const { return m_image; }
	protected:
		VulkanDevice* m_vulkanDevice;
		vk::Image m_image;
		MemoryRecord m_memoryRecord;
		MemoryRequirements m_requirements;
	
		bool m_scoped;
		bool m_cleanup;
		uint32_t m_width = 2;
		uint32_t m_height = 2;
		uint32_t m_depth = 1;
		uint32_t m_mips = 1;

		void BindMemory(MemoryPropertyFlags inMemoryPropertyFlags);
	};
	
	
}
