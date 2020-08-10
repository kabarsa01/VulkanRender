#pragma once

#include "vulkan/vulkan.hpp"
#include "objects/VulkanDevice.h"

using namespace VULKAN_HPP_NAMESPACE;

// samplers to be used as immutable samplers
class GlobalSamplers
{
public:
	Sampler repeatLinearMipLinear;
	Sampler repeatMirrorLinearMipLinear;
	Sampler borderBlackLinearMipLinear;
	Sampler borderWhiteLinearMipLinear;

	static GlobalSamplers* GetInstance() { return instance; }

	void Create(VulkanDevice* inVulkanDevice);
	void Destroy();

	inline void SetMipLodBias(float inMipLodBias) { mipLodBias = inMipLodBias; }

	std::vector<DescriptorSetLayoutBinding> GetBindings(uint32_t inStartIndex = 0);
private:
	static GlobalSamplers* instance;

	VulkanDevice* vulkanDevice;
	std::vector<DescriptorSetLayoutBinding> bindings;
	std::vector<Sampler*> samplers;
	float mipLodBias;

	GlobalSamplers();
	GlobalSamplers(const GlobalSamplers& inOther);
	virtual ~GlobalSamplers();
	void operator=(const GlobalSamplers& inOther);

	void ConstructBindings();
};
