#pragma once

#include "vulkan/vulkan.hpp"
#include "objects/VulkanDevice.h"

using namespace VULKAN_HPP_NAMESPACE;

// samplers to be used as immutable samplers
class GlobalSamplers
{
public:
	GlobalSamplers();
	virtual ~GlobalSamplers();

	void Create(VulkanDevice* inVulkanDevice);
	void Destroy();
private:
	VulkanDevice* vulkanDevice;

	Sampler repeatLinearMipLinear;
	Sampler repeatMirrorLinearMipLinear;
	Sampler borderBlackLinearMipLinear;
	Sampler borderWhiteLinearMipLinear;
};
