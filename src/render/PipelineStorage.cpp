#include "PipelineStorage.h"

PipelineStorage::PipelineStorage()
{

}

PipelineStorage::~PipelineStorage()
{

}

void PipelineStorage::DestroyPipelines(VulkanDevice* inDevice)
{
	Device& device = inDevice->GetDevice();
	for (auto& pair : pipelines)
	{
		device.destroyPipeline(pair.second);
	}
	pipelines.clear();
}

bool PipelineStorage::HasPipeline(HashString inHashString)
{
	return pipelines.find(inHashString) != pipelines.end();
}

bool PipelineStorage::StorePipeline(HashString inHashString, Pipeline inPipeline)
{
	if (pipelines.find(inHashString) != pipelines.end())
	{
		return false;
	}
	pipelines[inHashString] = inPipeline;
	return true;
}

Pipeline& PipelineStorage::GetPipeline(HashString inHashString)
{
	return pipelines[inHashString];
}

Pipeline& PipelineStorage::operator[](HashString inHashString)
{
	return pipelines[inHashString];
}

