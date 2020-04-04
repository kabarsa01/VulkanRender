#pragma once

#include "vulkan/vulkan.hpp"

using namespace VULKAN_HPP_NAMESPACE;

// Wrapper for a Vulkan image resource, for lifetime and possibly ownership control
class ImageWrapper
{
public:
	ImageWrapper();
	virtual ~ImageWrapper();
protected:

};


