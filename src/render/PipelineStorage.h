#pragma once

#include "vulkan/vulkan.hpp"
#include <map>
#include "common/HashString.h"
#include "objects/VulkanDevice.h"

using namespace VULKAN_HPP_NAMESPACE;

class PipelineStorage
{
public:
	PipelineStorage();
	virtual ~PipelineStorage();

	void DestroyPipelines(VulkanDevice* inDevice);

	bool HasPipeline(HashString inHashString);
	bool StorePipeline(HashString inHashString, Pipeline inPipeline);
	Pipeline& GetPipeline(HashString inHashString);

	Pipeline& operator[](HashString inHashString);
private:
	std::map<HashString, Pipeline> pipelines;
};