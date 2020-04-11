#pragma once

#include "vulkan/vulkan.hpp"
#include "../objects/VulkanDevice.h"
#include <vector>

using namespace VULKAN_HPP_NAMESPACE;

class VulkanPassBase
{
public:
	VulkanPassBase();
	virtual ~VulkanPassBase();

	void Create(VulkanDevice* inDevice);
	void Destroy();

	void Draw(CommandBuffer* inCmdBuffer);
protected:
	VulkanDevice* vulkanDevice;

	RenderPass renderPass;
	PipelineLayout pipelineLayout;
	Pipeline pipeline;

	std::vector<Framebuffer> framebuffers;
};
