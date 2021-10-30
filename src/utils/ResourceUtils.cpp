#include "ResourceUtils.h"
#include "data/Texture2D.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::Format;
	using VULKAN_HPP_NAMESPACE::ImageType;
	using VULKAN_HPP_NAMESPACE::SampleCountFlagBits;
	using VULKAN_HPP_NAMESPACE::ImageTiling;
	using VULKAN_HPP_NAMESPACE::ImageCreateFlags;
	using VULKAN_HPP_NAMESPACE::Extent3D;
	using VULKAN_HPP_NAMESPACE::ImageUsageFlagBits;
	
	vk::ImageSubresourceRange ResourceUtils::CreateColorSubresRange(uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
	{
		vk::ImageSubresourceRange range;

		range.setAspectMask(vk::ImageAspectFlagBits::eColor);
		range.setBaseMipLevel(baseMip);
		range.setLevelCount(mipCount);
		range.setBaseArrayLayer(baseLayer);
		range.setLayerCount(layerCount);

		return range;
	}

	vk::ImageSubresourceRange ResourceUtils::CreateColorSubresRange()
	{
		return CreateColorSubresRange(0, 1, 0, 1);
	}

	vk::ImageSubresourceRange ResourceUtils::CreateDepthSubresRange(uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
	{
		vk::ImageSubresourceRange range;

		range.setAspectMask(vk::ImageAspectFlagBits::eDepth);
		range.setBaseMipLevel(baseMip);
		range.setLevelCount(mipCount);
		range.setBaseArrayLayer(baseLayer);
		range.setLayerCount(layerCount);

		return range;
	}

	vk::ImageSubresourceRange ResourceUtils::CreateDepthSubresRange()
	{
		return CreateDepthSubresRange(0, 1, 0, 1);
	}

	vk::ImageViewType ResourceUtils::ImageTypeToViewType(vk::ImageType imageType)
	{
		vk::ImageViewType viewType;

		switch (imageType)
		{
		case vk::ImageType::e1D:
			viewType = vk::ImageViewType::e1D;
			break;
		case vk::ImageType::e2D:
			viewType = vk::ImageViewType::e2D;
			break;
		case vk::ImageType::e3D:
			viewType = vk::ImageViewType::e3D;
			break;
		default:
			viewType = vk::ImageViewType::e2D;
			break;
		}

		return viewType;
	}

	VulkanImage ResourceUtils::CreateImage2D(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight, vk::Format inFormat, vk::ImageUsageFlags inUsage)
	{
		VulkanImage image;

		image.createInfo.setArrayLayers(1);
		image.createInfo.setExtent(Extent3D(inWidth, inHeight, 1));
		image.createInfo.setFormat(inFormat);
		image.createInfo.setImageType(ImageType::e2D);
		image.createInfo.setInitialLayout(ImageLayout::eUndefined);
		image.createInfo.setMipLevels(1);
		image.createInfo.setSamples(SampleCountFlagBits::e1);
		image.createInfo.setSharingMode(SharingMode::eExclusive);
		image.createInfo.setTiling(ImageTiling::eOptimal);
		image.createInfo.setUsage(inUsage);
		image.Create(inDevice);
		image.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);

		return image;
	}

	VulkanImage ResourceUtils::CreateColorAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight, vk::Format format)
	{
		VulkanImage colorAttachmentImage;
		colorAttachmentImage.createInfo.setArrayLayers(1);
		colorAttachmentImage.createInfo.setFormat(format);
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
	
	VulkanImage ResourceUtils::CreateDepthAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight)
	{
		VulkanImage depthAttachmentImage;
		depthAttachmentImage.createInfo.setArrayLayers(1);
		depthAttachmentImage.createInfo.setFormat(Format::eD24UnormS8Uint);
		depthAttachmentImage.createInfo.setImageType(ImageType::e2D);
		depthAttachmentImage.createInfo.setInitialLayout(ImageLayout::eUndefined);
		depthAttachmentImage.createInfo.setSamples(SampleCountFlagBits::e1);
		depthAttachmentImage.createInfo.setMipLevels(1);
		depthAttachmentImage.createInfo.setSharingMode(SharingMode::eExclusive);
		depthAttachmentImage.createInfo.setTiling(ImageTiling::eOptimal);
		depthAttachmentImage.createInfo.setFlags(ImageCreateFlags());
		depthAttachmentImage.createInfo.setExtent(Extent3D(inWidth, inHeight, 1));
		depthAttachmentImage.createInfo.setUsage(ImageUsageFlagBits::eDepthStencilAttachment | ImageUsageFlagBits::eSampled | ImageUsageFlagBits::eTransferDst);
		depthAttachmentImage.Create(inDevice);
		depthAttachmentImage.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
	
		return depthAttachmentImage;
	}
	
	Texture2DPtr ResourceUtils::CreateColorAttachmentTexture(const HashString& name, VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight, vk::Format format /*= vk::Format::eR8G8B8A8Unorm*/)
	{
		VulkanImage image = ResourceUtils::CreateColorAttachment(inDevice, inWidth, inHeight, format);
		Texture2DPtr texture = std::make_shared<Texture2D>(name);
		texture->CreateFromExternal(image, image.CreateView(CreateColorSubresRange(), vk::ImageViewType::e2D), true);
		return texture;
	}

	Texture2DPtr ResourceUtils::CreateDepthAttachmentTexture(const HashString& name, VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight)
	{
		VulkanImage image = ResourceUtils::CreateDepthAttachment(inDevice, inWidth, inHeight);
		Texture2DPtr texture = std::make_shared<Texture2D>(name);
		texture->CreateFromExternal(image, image.CreateView(CreateDepthSubresRange(), vk::ImageViewType::e2D), true);
		return texture;
	}

	std::vector<Texture2DPtr> ResourceUtils::CreateColorAttachmentTextures(const HashString& name, uint32_t count, VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight, vk::Format format /*= vk::Format::eR8G8B8A8Unorm*/)
	{
		std::vector<Texture2DPtr> textures;
		textures.resize(count);
		for (uint32_t index = 0; index < count; ++index)
		{
			textures[index] = CreateColorAttachmentTexture(name + std::to_string(index), inDevice, inWidth, inHeight, format);
		}
		return textures;
	}

	std::vector<Texture2DPtr> ResourceUtils::CreateDepthAttachmentTextures(const HashString& name, uint32_t count, VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight)
	{
		std::vector<Texture2DPtr> textures;
		textures.resize(count);
		for (uint32_t index = 0; index < count; ++index)
		{
			textures[index] = CreateDepthAttachmentTexture(name + std::to_string(index), inDevice, inWidth, inHeight);
		}
		return textures;
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(const VulkanBuffer& buffer, const BindingInfo& info, vk::DescriptorBufferInfo& outDescInfo)
	{
		return CreateWriteDescriptor(buffer, info.descriptorType, info.binding, outDescInfo);
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(const VulkanBuffer& buffer, vk::DescriptorType type, uint32_t binding, vk::DescriptorBufferInfo& outDescInfo)
	{
		vk::WriteDescriptorSet write;

		outDescInfo = buffer.GetDescriptorInfo();

		write.setPBufferInfo(&outDescInfo);
		write.setDescriptorCount(1);
		write.setDescriptorType(type);
		write.setDstBinding(binding);

		return write;
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(const std::vector<VulkanBuffer>& buffers, const BindingInfo& info, std::vector<vk::DescriptorBufferInfo>& outDescInfo)
	{
		return CreateWriteDescriptor(buffers, info.descriptorType, info.binding, outDescInfo);
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(const std::vector<VulkanBuffer>& buffers, vk::DescriptorType type, uint32_t binding, std::vector<vk::DescriptorBufferInfo>& outDescInfo)
	{
		vk::WriteDescriptorSet write;

		for (auto& buffer : buffers)
		{
			outDescInfo.push_back(buffer.GetDescriptorInfo());
		}

		write.setPBufferInfo(outDescInfo.data());
		write.setDescriptorCount(static_cast<uint32_t>(buffers.size()));
		write.setDescriptorType(type);
		write.setDstBinding(binding);

		return write;
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(TextureDataPtr texture, const BindingInfo& info, vk::ImageLayout layout, vk::DescriptorImageInfo& outDescInfo)
	{
		return CreateWriteDescriptor(texture, info.descriptorType, info.binding, layout, outDescInfo);
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(TextureDataPtr texture, vk::DescriptorType type, uint32_t binding, vk::ImageLayout layout, vk::DescriptorImageInfo& outDescInfo)
	{
		vk::WriteDescriptorSet write;

		outDescInfo = { texture->GetDescriptorInfo(layout) };

		write.setPImageInfo(&outDescInfo);
		write.setDescriptorCount(1);
		write.setDescriptorType(type);
		write.setDstBinding(binding);

		return write;
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(const std::vector<TextureDataPtr>& textures, const BindingInfo& info, vk::ImageLayout layout, std::vector<vk::DescriptorImageInfo>& outDescInfo)
	{
		return CreateWriteDescriptor(textures, info.descriptorType, info.binding, layout, outDescInfo);
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(const std::vector<TextureDataPtr>& textures, vk::DescriptorType type, uint32_t binding, vk::ImageLayout layout, std::vector<vk::DescriptorImageInfo>& outDescInfo)
	{
		vk::WriteDescriptorSet write;

		for (auto texture : textures)
		{
			outDescInfo.push_back(texture->GetDescriptorInfo(layout));
		}

		write.setPImageInfo(outDescInfo.data());
		write.setDescriptorCount(static_cast<uint32_t>(textures.size()));
		write.setDescriptorType(type);
		write.setDstBinding(binding);

		return write;
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(const VulkanImage& image, const BindingInfo& info, vk::ImageLayout layout, vk::DescriptorImageInfo& outDescInfo)
	{
		return CreateWriteDescriptor(image, info.descriptorType, info.binding, layout, outDescInfo);
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(const VulkanImage& image, vk::DescriptorType type, uint32_t binding, vk::ImageLayout layout, vk::DescriptorImageInfo& outDescInfo)
	{
		vk::WriteDescriptorSet write;

		outDescInfo.setImageLayout(layout);
		outDescInfo.setImageView(image.CreateView(CreateColorSubresRange(), ImageTypeToViewType(image.createInfo.imageType)));

		write.setPImageInfo(&outDescInfo);
		write.setDescriptorCount(1);
		write.setDescriptorType(type);
		write.setDstBinding(binding);

		return write;
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(const vk::AccelerationStructureKHR& accelStruct, const BindingInfo& info, vk::WriteDescriptorSetAccelerationStructureKHR& outDescInfo)
	{
		return CreateWriteDescriptor(accelStruct, info.descriptorType, info.binding, outDescInfo);
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(const vk::AccelerationStructureKHR& accelStruct, vk::DescriptorType type, uint32_t binding, vk::WriteDescriptorSetAccelerationStructureKHR& outDescInfo)
	{
		vk::WriteDescriptorSet write;

		outDescInfo = vk::WriteDescriptorSetAccelerationStructureKHR();
		outDescInfo.setAccelerationStructureCount(1);
		outDescInfo.setPAccelerationStructures(&accelStruct);

		write.setDescriptorCount(1);
		write.setDescriptorType(type);
		write.setDstBinding(binding);
		write.setPNext(&outDescInfo);

		return write;
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(
		const std::vector<vk::AccelerationStructureKHR>& accelStructs, 
		const BindingInfo& info, 
		vk::WriteDescriptorSetAccelerationStructureKHR& outDescInfo)
	{
		return CreateWriteDescriptor(accelStructs, info.descriptorType, info.binding, outDescInfo);
	}

	vk::WriteDescriptorSet ResourceUtils::CreateWriteDescriptor(
		const std::vector<vk::AccelerationStructureKHR>& accelStructs, 
		vk::DescriptorType type, 
		uint32_t binding, 
		vk::WriteDescriptorSetAccelerationStructureKHR& outDescInfo)
	{
		vk::WriteDescriptorSet write;

		outDescInfo = vk::WriteDescriptorSetAccelerationStructureKHR();
		outDescInfo.setAccelerationStructureCount(static_cast<uint32_t>(accelStructs.size()));
		outDescInfo.setPAccelerationStructures(accelStructs.data());

		write.setDescriptorCount(static_cast<uint32_t>(accelStructs.size()));
		write.setDescriptorType(type);
		write.setDstBinding(binding);
		write.setPNext(&outDescInfo);

		return write;
	}

	VulkanBuffer ResourceUtils::CreateBuffer(VulkanDevice* inDevice, vk::DeviceSize inSize, vk::BufferUsageFlags inUsage, vk::MemoryPropertyFlags inMemProps, bool inWithStaging/* = false*/)
	{
		VulkanBuffer buffer;
		buffer.createInfo.setSharingMode(vk::SharingMode::eExclusive);
		buffer.createInfo.setSize(inSize);
		buffer.createInfo.setUsage(inUsage);
		buffer.Create(inDevice);
		buffer.BindMemory(inMemProps);
		if (inWithStaging)
		{
			buffer.CreateStagingBuffer();
		}

		return buffer;
	}

}
