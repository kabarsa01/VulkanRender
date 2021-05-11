#include "ImageUtils.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::Format;
	using VULKAN_HPP_NAMESPACE::ImageType;
	using VULKAN_HPP_NAMESPACE::SampleCountFlagBits;
	using VULKAN_HPP_NAMESPACE::ImageTiling;
	using VULKAN_HPP_NAMESPACE::ImageCreateFlags;
	using VULKAN_HPP_NAMESPACE::Extent3D;
	using VULKAN_HPP_NAMESPACE::ImageUsageFlagBits;
	
	VulkanImage ImageUtils::CreateColorAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight, bool in16BitFloat)
	{
		VulkanImage colorAttachmentImage;
		colorAttachmentImage.createInfo.setArrayLayers(1);
		colorAttachmentImage.createInfo.setFormat(in16BitFloat ? Format::eR16G16B16A16Sfloat : Format::eR8G8B8A8Unorm);
		colorAttachmentImage.createInfo.setImageType(ImageType::e2D);
		colorAttachmentImage.createInfo.setInitialLayout(ImageLayout::eUndefined);
		colorAttachmentImage.createInfo.setSamples(SampleCountFlagBits::e1);
		colorAttachmentImage.createInfo.setMipLevels(1);
		colorAttachmentImage.createInfo.setSharingMode(SharingMode::eExclusive);
		//colorAttachmentImage.createInfo.setQueueFamilyIndexCount(1);
		//colorAttachmentImage.createInfo.setPQueueFamilyIndices(queueFailyIndices);
		colorAttachmentImage.createInfo.setTiling(ImageTiling::eOptimal);
		colorAttachmentImage.createInfo.setFlags(ImageCreateFlags());
		colorAttachmentImage.createInfo.setExtent(Extent3D(inWidth, inHeight, 1));
		colorAttachmentImage.createInfo.setUsage(ImageUsageFlagBits::eColorAttachment | ImageUsageFlagBits::eSampled | ImageUsageFlagBits::eTransferDst);
		colorAttachmentImage.Create(inDevice);
		colorAttachmentImage.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
	
		return colorAttachmentImage;
	}
	
	VulkanImage ImageUtils::CreateDepthAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight)
	{
		VulkanImage depthAttachmentImage;
		depthAttachmentImage.createInfo.setArrayLayers(1);
		depthAttachmentImage.createInfo.setFormat(Format::eD24UnormS8Uint);
		depthAttachmentImage.createInfo.setImageType(ImageType::e2D);
		depthAttachmentImage.createInfo.setInitialLayout(ImageLayout::eUndefined);
		depthAttachmentImage.createInfo.setSamples(SampleCountFlagBits::e1);
		depthAttachmentImage.createInfo.setMipLevels(1);
		depthAttachmentImage.createInfo.setSharingMode(SharingMode::eExclusive);
		//colorAttachmentImage.createInfo.setQueueFamilyIndexCount(1);
		//colorAttachmentImage.createInfo.setPQueueFamilyIndices(queueFailyIndices);
		depthAttachmentImage.createInfo.setTiling(ImageTiling::eOptimal);
		depthAttachmentImage.createInfo.setFlags(ImageCreateFlags());
		depthAttachmentImage.createInfo.setExtent(Extent3D(inWidth, inHeight, 1));
		depthAttachmentImage.createInfo.setUsage(ImageUsageFlagBits::eDepthStencilAttachment | ImageUsageFlagBits::eSampled | ImageUsageFlagBits::eTransferDst);
		depthAttachmentImage.Create(inDevice);
		depthAttachmentImage.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
	
		return depthAttachmentImage;
	}
	
}
