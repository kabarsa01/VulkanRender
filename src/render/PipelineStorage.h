#pragma once

#include "vulkan/vulkan.hpp"
#include <map>
#include "common/HashString.h"

using namespace VULKAN_HPP_NAMESPACE;

class PipelineStorage
{
public:
	PipelineStorage();
	virtual ~PipelineStorage();

	bool HasPipeline(HashString inHashString);
	bool StorePipeline(HashString inHashString, Pipeline inPipeline);
private:
	std::map<HashString, Pipeline> pipelines;
};