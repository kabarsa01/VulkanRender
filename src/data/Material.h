#pragma once

#include "vulkan/vulkan.hpp"

#include "data/Resource.h"
#include "data/Texture2D.h"
#include "render/shader/Shader.h"
#include "render/objects/VulkanDescriptorSet.h"

using namespace VULKAN_HPP_NAMESPACE;

class VulkanDevice;

class Material : public Resource
{
public:
	Material(HashString inId);
	Material(HashString inId, const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath);
	virtual ~Material();

	void LoadResources();
	HashString GetShaderHash();

	void CreateDescriptorSet(VulkanDevice* inDevice);
	DescriptorSet GetDescriptorSet();
	std::vector<DescriptorSet> GetDescriptorSets();
	DescriptorSetLayout GetDescriptorSetLayout();

	PipelineShaderStageCreateInfo GetVertexStageInfo();
	PipelineShaderStageCreateInfo GetFragmentStageInfo();
	PipelineShaderStageCreateInfo GetComputeStageInfo();

	void SetEntrypoints(const std::string& inVertexEntrypoint, const std::string& inFragmentEntrypoint);
	void SetVertexEntrypoint(const std::string& inEntrypoint);
	void SetFragmentEntrypoint(const std::string& inEntrypoint);
	void SetComputeEntrypoint(const std::string& inEntrypoint);
	void SetShaderPath(const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath);
	void SetComputeShaderPath(const std::string& inComputeShaderPath);

	void SetTexture(const std::string& inName, Texture2DPtr inTexture2D);
	void SetStorageTexture(const std::string& inName, Texture2DPtr inTexture2D);
	template<typename T>
	void SetUniformBuffer(const std::string& inName, T& inUniformBuffer);
	template<typename T>
	void SetStorageBuffer(const std::string& inName, T& inStorageBuffer);
	void SetUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData);
	void SetStorageBuffer(const std::string& inName, uint64_t inSize, const char* inData);
	template<typename T>
	void UpdateUniformBuffer(const std::string& inName, T& inUniformBuffer);
	void UpdateUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData);
	void UpdateDescriptorSet(DescriptorSet inSet, VulkanDevice* inDevice);

	inline ShaderPtr GetVertexShader() { return vertexShader; }
	inline ShaderPtr GetFragmentShader() { return fragmentShader; }
	inline ShaderPtr GetComputeShader() { return computeShader; }
	inline const std::string& GetVertexEntrypoint() const { return vertexEntrypoint; };
	inline const std::string& GetFragmentEntrypoint() const { return fragmentEntrypoint; };
	inline const std::string& GetComputeEntrypoint() const { return computeEntrypoint; };

	bool Load() override;
	bool Cleanup() override;

	std::vector<DescriptorSetLayoutBinding>& GetBindings();
	std::vector<WriteDescriptorSet>& GetDescriptorWrites();
protected:
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
	std::string computeShaderPath;
	std::string vertexEntrypoint;
	std::string fragmentEntrypoint;
	std::string computeEntrypoint;
	HashString shaderHash;

	ShaderPtr vertexShader;
	ShaderPtr fragmentShader;
	ShaderPtr computeShader;
	std::map<HashString, Texture2DPtr> sampledImages2D;
	std::map<HashString, Texture2DPtr> storageImages2D;
	std::map<HashString, VulkanBuffer> buffers;
	std::map<HashString, VulkanBuffer> storageBuffers;

	std::vector<DescriptorSetLayoutBinding> descriptorBindings;
	std::map<HashString, DescriptorImageInfo> imageDescInfos;
	std::map<HashString, DescriptorBufferInfo> bufferDescInfos;
	std::vector<WriteDescriptorSet> descriptorWrites;

	VulkanDevice* vulkanDevice;
	VulkanDescriptorSet vulkanDescriptorSet;

	void PrepareDescriptorInfos();

	ShaderPtr InitShader(const std::string& inResourcePath);
	void PrepareDescriptorWrites(ShaderPtr inShader);

	template<class T>
	void ProcessDescriptorType(DescriptorType inType, ShaderPtr inShader, std::map<HashString, T>& inResources, std::vector<DescriptorSetLayoutBinding>& inOutBindings);
	template<class T>
	void PrepareDescriptorWrites(DescriptorType inType, ShaderPtr inShader, std::map<HashString, T>& inDescInfos, std::vector<WriteDescriptorSet>& inOutDescriptorWrites);
};

typedef std::shared_ptr<Material> MaterialPtr;

//======================================================================================================================================================
// DEFINITIONS
//======================================================================================================================================================

template<typename T>
void Material::SetUniformBuffer(const std::string& inName, T& inUniformBuffer)
{
	SetUniformBuffer(inName, sizeof(T), reinterpret_cast<const char*>(&inUniformBuffer));
}

template<typename T>
void Material::SetStorageBuffer(const std::string& inName, T& inStorageBuffer)
{
	SetStorageBuffer(inName, sizeof(T), reinterpret_cast<const char*>(&inStorageBuffer));
}

template<typename T>
void Material::UpdateUniformBuffer(const std::string& inName, T& inUniformBuffer)
{
	UpdateUniformBuffer(inName, sizeof(T), reinterpret_cast<const char*>(&inUniformBuffer));
}

template<class T>
void Material::ProcessDescriptorType(DescriptorType inType, ShaderPtr inShader, std::map<HashString, T>& inResources, std::vector<DescriptorSetLayoutBinding>& inOutBindings)
{
	std::vector<BindingInfo>& bindingInfoVector = inShader->GetBindings(inType);
	for (BindingInfo& info : bindingInfoVector)
	{
		if (inResources.find(info.name) == inResources.end())
		{
			continue;
		}

		DescriptorSetLayoutBinding binding;
		binding.setBinding(info.binding);
		binding.setDescriptorType(info.descriptorType);
		binding.setDescriptorCount(info.IsArray() ? info.arrayDimensions[0] : 1);
		binding.setStageFlags(ShaderStageFlagBits::eAllGraphics | ShaderStageFlagBits::eCompute);

		inOutBindings.push_back(binding);
	}
}

template<class T>
void Material::PrepareDescriptorWrites(DescriptorType inType, ShaderPtr inShader, std::map<HashString, T>& inDescInfos, std::vector<WriteDescriptorSet>& inOutDescriptorWrites)
{
	std::vector<BindingInfo>& bindingInfoVector = inShader->GetBindings(inType);
	for (BindingInfo& info : bindingInfoVector)
	{
		if (inDescInfos.find(info.name) == inDescInfos.end())
		{
			continue;
		}

		WriteDescriptorSet writeDescriptorSet;
		// we do not know the set at the moment
		//writeDescriptorSet.setDstSet(descriptorSet);
		writeDescriptorSet.setDstBinding(info.binding);
		writeDescriptorSet.setDstArrayElement(0);
		writeDescriptorSet.setDescriptorCount(1);
		writeDescriptorSet.setDescriptorType(info.descriptorType);

		switch (info.descriptorType)
		{
		case DescriptorType::eSampledImage:
			writeDescriptorSet.setPImageInfo(& imageDescInfos[info.name]);
			break;
		case DescriptorType::eStorageImage:
			writeDescriptorSet.setPImageInfo(&imageDescInfos[info.name]);
			break;
		case DescriptorType::eUniformBuffer:
			writeDescriptorSet.setPBufferInfo(& bufferDescInfos[info.name]);
			break;
		case DescriptorType::eStorageBuffer:
			writeDescriptorSet.setPBufferInfo(&bufferDescInfos[info.name]);
			break;
		}

		inOutDescriptorWrites.push_back(writeDescriptorSet);
	}
}

