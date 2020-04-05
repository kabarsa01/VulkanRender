#pragma once

#include "vulkan/vulkan.hpp"

using namespace VULKAN_HPP_NAMESPACE;

// Wrapper for a Vulkan image resource, for lifetime and possibly ownership control
class ImageWrapper
{
public:
	ImageCreateInfo createInfo;

	ImageWrapper();
	virtual ~ImageWrapper();

	void Create();
	void Destroy();

	Image& GetImage();
	Image GetImage() const;
	MemoryRequirements GetMemoryRequirements();

	operator Image() const { return image; }
	operator bool() const { return image; }
protected:
	Device device;
	Image image;

	//uint32_t width;
	//uint32_t height;
	//uint32_t depth;
	//uint32_t mipLevels;
	//uint32_t arrayLayers;
	//Format format;
	//ImageTiling tiling;
	//ImageLayout initialLayout;
	//ImageUsageFlags usage;
	//SampleCountFlagBits samples;
	//SharingMode sharingMode;
};


