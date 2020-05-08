#pragma once

#include "vulkan/vulkan.hpp"
#include <map>
#include "common/HashString.h"
#include "objects/VulkanDevice.h"
#include "objects/VulkanDescriptorSet.h"

using namespace VULKAN_HPP_NAMESPACE;

struct PipelineData
{
	Pipeline pipeline;
	PipelineLayout pipelineLayout;
	VulkanDescriptorSet vulkanDescriptorSet;
	std::vector<DescriptorSet> descriptorSets;
};

class PipelineRegistry
{
public:
	static PipelineRegistry* GetInstance();

	void DestroyPipelines(VulkanDevice* inDevice);

	bool HasPipeline(HashString inShadersHash, HashString inPassHash);
	bool StorePipeline(HashString inShadersHash, HashString inPassHash, PipelineData inPipelineData);
	PipelineData& GetPipeline(HashString inShadersHash, HashString inPassHash);

	std::map<HashString, PipelineData>& operator[](HashString inShaderHash);
private:
	static PipelineRegistry* instance;

	//std::map<HashString, Pipeline> pipelines;
	// shader hash - render pass hash - pipeline data
	std::map<HashString, std::map<HashString, PipelineData>> pipelinesData;

	PipelineRegistry();
	PipelineRegistry(const PipelineRegistry& inOther);
	virtual ~PipelineRegistry();
	void operator=(const PipelineRegistry& inOther);
};