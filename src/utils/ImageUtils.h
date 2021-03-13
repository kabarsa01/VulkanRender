#pragma once

#include "render/resources/VulkanImage.h"

namespace CGE
{
	class VulkanDevice;
	
	class ImageUtils
	{
	public:
		static VulkanImage CreateColorAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight, bool in16BitFloat = false);
		static VulkanImage CreateDepthAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight);
	};
}
