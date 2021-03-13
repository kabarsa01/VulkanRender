#pragma once

#include "vulkan/vulkan.hpp"
#include <map>
#include "common/HashString.h"
#include "objects/VulkanDevice.h"
#include "objects/VulkanDescriptorSet.h"


namespace CGE
{
	namespace vk = VULKAN_HPP_NAMESPACE;

	struct PipelineData
	{
		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		std::vector<DescriptorSet> descriptorSets;
	};
	
	class PipelineRegistry
	{
	public:
		static PipelineRegistry* GetInstance();
	
		void DestroyPipelines(VulkanDevice* inDevice);
		void DestroyPipelines(VulkanDevice* inDevice, HashString inPassHash);
	
		bool HasPipeline(HashString inPassHash, HashString inShadersHash);
		bool StorePipeline(HashString inPassHash, HashString inShadersHash, PipelineData inPipelineData);
		PipelineData& GetPipeline(HashString inPassHash, HashString inShadersHash);
	
		std::map<HashString, PipelineData>& operator[](HashString inPassHash);
	private:
		static PipelineRegistry* instance;
	
		//std::map<HashString, Pipeline> pipelines;
		// render pass hash - shader hash - pipeline data
		std::map<HashString, std::map<HashString, PipelineData>> pipelinesData;
	
		PipelineRegistry();
		PipelineRegistry(const PipelineRegistry& inOther);
		virtual ~PipelineRegistry();
		void operator=(const PipelineRegistry& inOther);
	};
}