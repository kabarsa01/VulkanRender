#pragma once

#include "render/resources/VulkanImage.h"
#include "render/shader/Shader.h"
#include "data/TextureData.h"

namespace CGE
{
	class VulkanDevice;
	
	class ResourceUtils
	{
	public:
		static vk::ImageSubresourceRange CreateColorSubresRange(uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount);
		static vk::ImageSubresourceRange CreateColorSubresRange();
		static vk::ImageSubresourceRange CreateDepthSubresRange(uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount);
		static vk::ImageSubresourceRange CreateDepthSubresRange();

		static vk::ImageViewType ImageTypeToViewType(vk::ImageType imageType);

		static VulkanImage CreateImage2D(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight, vk::Format inFormat, vk::ImageUsageFlags inUsage);
		static VulkanImage CreateColorAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight, vk::Format format = vk::Format::eR8G8B8A8Unorm);
		static VulkanImage CreateDepthAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight);

		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const VulkanBuffer& buffer, const BindingInfo& info, vk::DescriptorBufferInfo& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const VulkanBuffer& buffer, vk::DescriptorType type, uint32_t binding, vk::DescriptorBufferInfo& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const std::vector<VulkanBuffer>& buffers, 
			const BindingInfo& info, 
			std::vector<vk::DescriptorBufferInfo>& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const std::vector<VulkanBuffer>& buffers, 
			vk::DescriptorType type, 
			uint32_t binding, 
			std::vector<vk::DescriptorBufferInfo>& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			TextureDataPtr texture, 
			const BindingInfo& info, 
			vk::ImageLayout layout, 
			vk::DescriptorImageInfo& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			TextureDataPtr texture, 
			vk::DescriptorType type, 
			uint32_t binding, 
			vk::ImageLayout layout, 
			vk::DescriptorImageInfo& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const std::vector<TextureDataPtr>& textures, 
			const BindingInfo& info, 
			vk::ImageLayout layout, 
			std::vector<vk::DescriptorImageInfo>& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const std::vector<TextureDataPtr>& textures, 
			vk::DescriptorType type, 
			uint32_t binding, 
			vk::ImageLayout layout, 
			std::vector<vk::DescriptorImageInfo>& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const VulkanImage& image, 
			const BindingInfo& info, 
			vk::ImageLayout layout,
			vk::DescriptorImageInfo& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const VulkanImage& image, 
			vk::DescriptorType type, 
			uint32_t binding, 
			vk::ImageLayout layout, 
			vk::DescriptorImageInfo& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const vk::AccelerationStructureKHR& accelStruct, 
			const BindingInfo& info, 
			vk::WriteDescriptorSetAccelerationStructureKHR& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const vk::AccelerationStructureKHR& accelStruct, 
			vk::DescriptorType type, 
			uint32_t binding, 
			vk::WriteDescriptorSetAccelerationStructureKHR& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const std::vector<vk::AccelerationStructureKHR>& accelStructs, 
			const BindingInfo& info, 
			vk::WriteDescriptorSetAccelerationStructureKHR& outDescInfo);
		static vk::WriteDescriptorSet CreateWriteDescriptor(
			const std::vector<vk::AccelerationStructureKHR>& accelStructs, 
			vk::DescriptorType type, 
			uint32_t binding, 
			vk::WriteDescriptorSetAccelerationStructureKHR& outDescInfo);

		static VulkanBuffer CreateBuffer(VulkanDevice* inDevice, vk::DeviceSize inSize, vk::BufferUsageFlags inUsage, vk::MemoryPropertyFlags inMemProps, bool inWithStaging = false);
	};
}
