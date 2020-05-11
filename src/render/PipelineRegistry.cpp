#include "PipelineRegistry.h"

PipelineRegistry* PipelineRegistry::instance = new PipelineRegistry();

PipelineRegistry::PipelineRegistry()
{
}

PipelineRegistry::PipelineRegistry(const PipelineRegistry& inOther)
{
}

PipelineRegistry::~PipelineRegistry()
{
}

void PipelineRegistry::operator=(const PipelineRegistry& inOther)
{
}

PipelineRegistry* PipelineRegistry::GetInstance()
{
	return instance;
}

void PipelineRegistry::DestroyPipelines(VulkanDevice* inDevice)
{
	Device& device = inDevice->GetDevice();
	for (auto& shaderPair : pipelinesData)
	{
		for (auto& passPair : shaderPair.second)
		{
			device.destroyPipelineLayout(passPair.second.pipelineLayout);
			device.destroyPipeline(passPair.second.pipeline);
		}
	}
	pipelinesData.clear();
}

bool PipelineRegistry::HasPipeline(HashString inShadersHash, HashString inPassHash)
{
	if (pipelinesData.find(inShadersHash) != pipelinesData.end())
	{
		std::map<HashString, PipelineData>& passData = pipelinesData[inShadersHash];
		return passData.find(inPassHash) != passData.end();
	}
	return false;
}

bool PipelineRegistry::StorePipeline(HashString inShadersHash, HashString inPassHash, PipelineData inPipelineData)
{
	pipelinesData[inShadersHash][inPassHash] = inPipelineData;
	return true;
}

PipelineData& PipelineRegistry::GetPipeline(HashString inShadersHash, HashString inPassHash)
{
	return pipelinesData[inShadersHash][inPassHash];
}

std::map<HashString, PipelineData>& PipelineRegistry::operator[](HashString inShaderHash)
{
	return pipelinesData[inShaderHash];
}

