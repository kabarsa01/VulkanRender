#include "PipelineStorage.h"

PipelineStorage::PipelineStorage()
{

}

PipelineStorage::~PipelineStorage()
{

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

