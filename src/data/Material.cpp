#include "Material.h"
#include "DataManager.h"
#include "core/Engine.h"

Material::Material(HashString inId)
	: Resource(inId)
	, shaderHash(HashString::NONE)
{

}

Material::Material(HashString inId, const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath)
	: Resource(inId)
	, vertexShaderPath(inVertexShaderPath)
	, fragmentShaderPath(inFragmentShaderPath)
	, shaderHash(inVertexShaderPath + inFragmentShaderPath)
{

}

Material::~Material()
{

}

void Material::LoadResources()
{
	vertexShader = DataManager::RequestResourceType<Shader>(vertexShaderPath);
	fragmentShader = DataManager::RequestResourceType<Shader>(fragmentShaderPath);

	// TODO: process all the material resources
	descriptorBindings.clear();
	ProcessDescriptorType<Texture2DPtr>(DescriptorType::eSampledImage, vertexShader, textures2D, descriptorBindings);
	ProcessDescriptorType<Texture2DPtr>(DescriptorType::eSampledImage, fragmentShader, textures2D, descriptorBindings);
	ProcessDescriptorType<VulkanBuffer>(DescriptorType::eUniformBuffer, vertexShader, buffers, descriptorBindings);
	ProcessDescriptorType<VulkanBuffer>(DescriptorType::eUniformBuffer, fragmentShader, buffers, descriptorBindings);

	PrepareDescriptorInfos();

	descriptorWrites.clear();
	PrepareDescriptorWrites<DescriptorImageInfo>(DescriptorType::eSampledImage, vertexShader, imageDescInfos, descriptorWrites);
	PrepareDescriptorWrites<DescriptorImageInfo>(DescriptorType::eSampledImage, fragmentShader, imageDescInfos, descriptorWrites);
	PrepareDescriptorWrites<DescriptorBufferInfo>(DescriptorType::eUniformBuffer, vertexShader, bufferDescInfos, descriptorWrites);
	PrepareDescriptorWrites<DescriptorBufferInfo>(DescriptorType::eUniformBuffer, fragmentShader, bufferDescInfos, descriptorWrites);
}

HashString Material::GetShaderHash()
{
	return shaderHash;
}

void Material::SetShaderPath(const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath)
{
	vertexShaderPath = inVertexShaderPath;
	fragmentShaderPath = inFragmentShaderPath;
	shaderHash = HashString(vertexShaderPath + fragmentShaderPath);
}

void Material::SetTexture(const std::string& inName, Texture2DPtr inTexture2D)
{
	textures2D[inName] = inTexture2D;
}

void Material::SetUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData)
{
	VulkanDevice& vulkanDevice = Engine::GetRendererInstance()->GetVulkanDevice();

	VulkanBuffer buffer;
	buffer.createInfo.setSharingMode(SharingMode::eExclusive);
	buffer.createInfo.setSize(inSize);
	buffer.createInfo.setUsage(BufferUsageFlagBits::eUniformBuffer);
	buffer.Create(&vulkanDevice);
	buffer.BindMemory(MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent);

	buffers[inName].Destroy();
	buffers[inName] = buffer;

	UpdateUniformBuffer(inName, inSize, inData);
}

void Material::UpdateUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData)
{
	buffers[inName].CopyTo(inSize, inData);
}

bool Material::Load()
{
	return true;
}

bool Material::Cleanup()
{
	// TODO: cleanup at least buffers
	for (auto& pair : buffers)
	{
		pair.second.Destroy();
	}
	return true;
}

std::vector<DescriptorSetLayoutBinding>& Material::GetBindings()
{
	return descriptorBindings;
}

std::vector<WriteDescriptorSet>& Material::GetDescriptorWrites()
{
	return descriptorWrites;
}

void Material::PrepareDescriptorInfos()
{
	for (auto& pair : textures2D)
	{
		DescriptorImageInfo imageInfo;
		imageInfo.setImageView(pair.second->GetImageView());
		imageInfo.setImageLayout(ImageLayout::eShaderReadOnlyOptimal);

		imageDescInfos[pair.first] = imageInfo;
	}

	for (auto& pair : buffers)
	{
		bufferDescInfos[pair.first] = pair.second.GetDescriptorInfo();
	}
}


