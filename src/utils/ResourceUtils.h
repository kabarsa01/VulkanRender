#pragma once

#include "render/resources/VulkanImage.h"

namespace CGE
{
	class VulkanDevice;
	
	class ResourceUtils
	{
	public:
		static VulkanImage CreateColorAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight, bool in16BitFloat = false);
		static VulkanImage CreateDepthAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight);

		static VulkanBuffer CreateBuffer(VulkanDevice* inDevice, vk::DeviceSize inSize, vk::BufferUsageFlags inUsage, vk::MemoryPropertyFlags inMemProps, bool inWithStaging = false);
	};
}
