#pragma once

#include "vulkan/vulkan.hpp"
#include "../objects/VulkanDevice.h"
#include "../memory/DeviceMemoryManager.h"
#include "VulkanBuffer.h"
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
	
		void Create(VulkanDevice* inDevice);
		ImageView CreateView(ImageSubresourceRange inSubRange, ImageViewType inViewType) const;
		void Destroy();
	
		inline uint32_t GetWidth() { return width; }
		inline uint32_t GetHeight() { return height; }
		inline uint32_t GetDepth() { return depth; }
		inline uint32_t GetMips() { return mips; }
	
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
		VulkanBuffer* CreateStagingBuffer(char* inData);
		VulkanBuffer* CreateStagingBuffer(SharingMode inSharingMode, uint32_t inQueueFamilyIndex);
		VulkanBuffer* CreateStagingBuffer(SharingMode inSharingMode, uint32_t inQueueFamilyIndex, char* inData);
		void DestroyStagingBuffer();
	
		operator Image() const { return image; }
		operator bool() const { return image; }
	protected:
		VulkanDevice* vulkanDevice;
		Image image;
		MemoryRecord memoryRecord;
		MemoryRequirements memoryRequirements;
		VulkanBuffer stagingBuffer;
		std::vector<char> data;
	
		bool scoped;
		uint32_t width = 2;
		uint32_t height = 2;
		uint32_t depth = 1;
		uint32_t mips = 1;
	};
	
	
}
