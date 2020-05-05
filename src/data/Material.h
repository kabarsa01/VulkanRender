#pragma once

#include "data/Resource.h"
#include "data/Texture2D.h"
#include "render/shader/Shader.h"
#include "core/Engine.h"

class Material : public Resource
{
public:
	Material(HashString inId);
	Material(HashString inId, const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath);
	virtual ~Material();

	void LoadResources();
	HashString GetShaderHash();

	void SetShaderPath(const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath);
	void SetTexture(const std::string& inName, Texture2DPtr inTexture2D);
	template<typename T>
	void SetUniformBuffer(const std::string& inName, T& inUniformBuffer);
	template<typename T>
	void UpdateUniformBuffer(const std::string& inName, T& inUniformBuffer);

	inline ShaderPtr GetVertexShader() { return vertexShader; }
	inline ShaderPtr GetFragmentShader() { return fragmentShader; }

	bool Load() override;
	bool Cleanup() override;

	std::vector<DescriptorSetLayoutBinding>& GetBindings();
	std::vector<WriteDescriptorSet>& GetDescriptorWrites();
protected:
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
	HashString shaderHash;

	ShaderPtr vertexShader;
	ShaderPtr fragmentShader;
	std::map<HashString, Texture2DPtr> textures2D;
	std::map<HashString, VulkanBuffer> buffers;

	std::vector<DescriptorSetLayoutBinding> descriptorBindings;
	std::map<HashString, DescriptorImageInfo> imageDescInfos;
	std::map<HashString, DescriptorBufferInfo> bufferDescInfos;
	std::vector<WriteDescriptorSet> descriptorWrites;

	void PrepareDescriptorInfos();

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
	VulkanDevice& vulkanDevice = Engine::GetRendererInstance()->GetVulkanDevice();

	VulkanBuffer buffer;
	buffer.createInfo.setSharingMode(SharingMode::eExclusive);
	buffer.createInfo.setSize(sizeof(T));
	buffer.createInfo.setUsage(BufferUsageFlagBits::eUniformBuffer);
	buffer.Create(&vulkanDevice);
	buffer.BindMemory(MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent);

	buffers[inName].Destroy();
	buffers[inName] = buffer;

	UpdateUniformBuffer(inName, inUniformBuffer);
}

template<typename T>
void Material::UpdateUniformBuffer(const std::string& inName, T& inUniformBuffer)
{
	buffers[inName].CopyTo(sizeof(T), reinterpret_cast<char*>(&inUniformBuffer));
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
		binding.setStageFlags(ShaderStageFlagBits::eAllGraphics);

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
		case DescriptorType::eUniformBuffer:
			writeDescriptorSet.setPBufferInfo(& bufferDescInfos[info.name]);
			break;
		}

		inOutDescriptorWrites.push_back(writeDescriptorSet);
	}
}

