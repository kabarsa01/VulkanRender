#pragma once

#include "vulkan/vulkan.hpp"

using namespace VULKAN_HPP_NAMESPACE;

// Wrapper for a Vulkan image resource, for lifetime and possibly ownership control
class VulkanImage
{
public:
	ImageCreateInfo createInfo;

	VulkanImage();
	virtual ~VulkanImage();

	void Create();
	void Destroy();

	Image& GetImage();
	const Image& GetImage() const;
	MemoryRequirements GetMemoryRequirements();

	operator Image() const { return image; }
	operator bool() const { return image; }
protected:
	Device device;
	Image image;
};


