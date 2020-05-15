#pragma once

#include "render/resources/VulkanImage.h"

class VulkanDevice;

class ImageUtils
{
public:
	static VulkanImage CreateColorAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight);
	static VulkanImage CreateDepthAttachment(VulkanDevice* inDevice, uint32_t inWidth, uint32_t inHeight);
};
