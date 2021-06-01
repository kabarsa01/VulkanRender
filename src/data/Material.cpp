#include "Material.h"
#include "DataManager.h"
#include "core/Engine.h"
#include "render/Renderer.h"
#include "render/TransferList.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::ShaderStageFlagBits;
	using VULKAN_HPP_NAMESPACE::BufferUsageFlagBits;

	Material::Material(HashString inId)
		: Resource(inId)
		, vertexEntrypoint("main")
		, fragmentEntrypoint("main")
		, computeEntrypoint("main")
	{
	
	}
	
	Material::Material(HashString inId, const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath)
		: Resource(inId)
		, vertexShaderPath(inVertexShaderPath)
		, fragmentShaderPath(inFragmentShaderPath)
		, vertexEntrypoint("main")
		, fragmentEntrypoint("main")
		, computeEntrypoint("main")
	{
	
	}
	
	Material::~Material()
	{
	
	}
	
	void Material::LoadResources()
	{
		// TODO: process all the material resources
		descriptorBindings.clear();
		descriptorWrites.clear();
	
		vertexShader = InitShader(vertexShaderPath);
		fragmentShader = InitShader(fragmentShaderPath);
		computeShader = InitShader(computeShaderPath);
	
		PrepareDescriptorInfos();
	
		PrepareDescriptorWrites(vertexShader);
		PrepareDescriptorWrites(fragmentShader);
		PrepareDescriptorWrites(computeShader);
	
		shaderHash = HashString(vertexShaderPath + fragmentShaderPath + computeShaderPath);
	}
	
	HashString Material::GetShaderHash()
	{
		return shaderHash;
	}
	
	void Material::CreateDescriptorSet(VulkanDevice* inDevice)
	{
		if (vulkanDescriptorSet)
		{
			return;
		}
	
		vulkanDevice = inDevice;
		vulkanDescriptorSet.SetBindings(GetBindings());
		vulkanDescriptorSet.Create(vulkanDevice);
		UpdateDescriptorSet(vulkanDescriptorSet.GetSet(), vulkanDevice);
	}
	
	DescriptorSet Material::GetDescriptorSet()
	{
		return vulkanDescriptorSet.GetSet();
	}
	
	std::vector<DescriptorSet> Material::GetDescriptorSets()
	{
		return { vulkanDescriptorSet.GetSet() };
	}
	
	DescriptorSetLayout Material::GetDescriptorSetLayout()
	{
		return vulkanDescriptorSet.GetLayout();
	}
	
	PipelineShaderStageCreateInfo Material::GetVertexStageInfo()
	{
		PipelineShaderStageCreateInfo vertStageInfo;
		vertStageInfo.setStage(ShaderStageFlagBits::eVertex);
		vertStageInfo.setModule(vertexShader->GetShaderModule());
		vertStageInfo.setPName(vertexEntrypoint.c_str());
	
		return vertStageInfo;
	}
	
	PipelineShaderStageCreateInfo Material::GetFragmentStageInfo()
	{
		PipelineShaderStageCreateInfo fragStageInfo;
		fragStageInfo.setStage(ShaderStageFlagBits::eFragment);
		fragStageInfo.setModule(fragmentShader->GetShaderModule());
		fragStageInfo.setPName(fragmentEntrypoint.c_str());
	
		return fragStageInfo;
	}
	
	PipelineShaderStageCreateInfo Material::GetComputeStageInfo()
	{
		PipelineShaderStageCreateInfo computeStageInfo;
		computeStageInfo.setStage(ShaderStageFlagBits::eCompute);
		computeStageInfo.setModule(computeShader->GetShaderModule());
		computeStageInfo.setPName(computeEntrypoint.c_str());
	
		return computeStageInfo;
	}
	
	void Material::SetEntrypoints(const std::string& inVertexEntrypoint, const std::string& inFragmentEntrypoint)
	{
		vertexEntrypoint = inVertexEntrypoint;
		fragmentEntrypoint = inFragmentEntrypoint;
	}
	
	void Material::SetVertexEntrypoint(const std::string& inEntrypoint)
	{
		vertexEntrypoint = inEntrypoint;
	}
	
	void Material::SetFragmentEntrypoint(const std::string& inEntrypoint)
	{
		fragmentEntrypoint = inEntrypoint;
	}
	
	void Material::SetComputeEntrypoint(const std::string& inEntrypoint)
	{
		computeEntrypoint = inEntrypoint;
	}
	
	void Material::SetShaderPath(const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath)
	{
		vertexShaderPath = inVertexShaderPath;
		fragmentShaderPath = inFragmentShaderPath;
	}
	
	void Material::SetComputeShaderPath(const std::string& inComputeShaderPath)
	{
		computeShaderPath = inComputeShaderPath;
	}
	
	void Material::SetTexture(const std::string& inName, Texture2DPtr inTexture2D)
	{
		sampledImages2D[inName] = inTexture2D;
	}
	
	void Material::SetStorageTexture(const std::string& inName, Texture2DPtr inTexture2D)
	{
		storageImages2D[inName] = inTexture2D;
	}
	
	void Material::SetUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData)
	{
		VulkanDevice& vulkanDevice = Engine::GetRendererInstance()->GetVulkanDevice();
	
		VulkanBuffer buffer;
		buffer.createInfo.setSharingMode(SharingMode::eExclusive);
		buffer.createInfo.setSize(inSize);
		buffer.createInfo.setUsage(BufferUsageFlagBits::eUniformBuffer | BufferUsageFlagBits::eTransferDst);
		buffer.Create(&vulkanDevice);
		buffer.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
		buffer.CreateStagingBuffer();
	
		buffers[inName].Destroy();
		buffers[inName] = buffer;
	
		UpdateUniformBuffer(inName, inSize, inData);
	}
	
	void Material::SetUniformBufferExternal(const std::string& inName, const VulkanBuffer& inBuffer)
	{
		buffers[inName].Destroy();
		buffers[inName] = inBuffer;
		buffers[inName].SetCleanup(false);
	}
	
	void Material::SetStorageBufferExternal(const std::string& inName, const VulkanBuffer& inBuffer)
	{
		storageBuffers[inName].Destroy();
		storageBuffers[inName] = inBuffer;
		storageBuffers[inName].SetCleanup(false);
	}
	
	void Material::SetStorageBuffer(const std::string& inName, uint64_t inSize, const char* inData)
	{
		VulkanDevice& vulkanDevice = Engine::GetRendererInstance()->GetVulkanDevice();
	
		VulkanBuffer buffer(false);
		buffer.createInfo.setSharingMode(SharingMode::eExclusive);
		buffer.createInfo.setSize(inSize);
		buffer.createInfo.setUsage(BufferUsageFlagBits::eStorageBuffer | BufferUsageFlagBits::eTransferDst);
		buffer.Create(&vulkanDevice);
		buffer.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
		buffer.CreateStagingBuffer();
	
		storageBuffers[inName].Destroy();
		storageBuffers[inName] = buffer;
	}
	
	void Material::UpdateUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData)
	{
		buffers[inName].CopyTo(inSize, inData);
		TransferList::GetInstance()->PushBuffer(&buffers[inName]);
	}
	
	void Material::UpdateStorageBuffer(const std::string& inName, uint64_t inSize, const char* inData)
	{
		storageBuffers[inName].CopyTo(inSize, inData);
		TransferList::GetInstance()->PushBuffer(&storageBuffers[inName]);
	}
	
	void Material::UpdateDescriptorSet(DescriptorSet inSet, VulkanDevice* inDevice)
	{
		std::vector<WriteDescriptorSet>& writes = GetDescriptorWrites();
		for (WriteDescriptorSet& write : writes)
		{
			write.setDstSet(inSet);
		}
		inDevice->GetDevice().updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}
	
	VulkanBuffer& Material::GetUniformBuffer(const std::string& inName)
	{
		return buffers[inName];
	}
	
	VulkanBuffer& Material::GetStorageBuffer(const std::string& inName)
	{
		return storageBuffers[inName];
	}
	
	bool Material::Create()
	{
		return true;
	}
	
	bool Material::Destroy()
	{
		// cleanup buffers. textures are resources themselves and will be cleaned by data manager
		for (auto& pair : buffers)
		{
			pair.second.Destroy();
		}
		for (auto& pair : storageBuffers)
		{
			pair.second.Destroy();
		}
		vulkanDescriptorSet.Destroy();
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
		for (auto& pair : sampledImages2D)
		{
			DescriptorImageInfo imageInfo;
			imageInfo.setImageView(pair.second->GetImageView());
			imageInfo.setImageLayout(ImageLayout::eShaderReadOnlyOptimal);
	
			imageDescInfos[pair.first] = imageInfo;
		}
		for (auto& pair : storageImages2D)
		{
			DescriptorImageInfo imageInfo;
			imageInfo.setImageView(pair.second->GetImageView());
			imageInfo.setImageLayout(ImageLayout::eGeneral);
	
			imageDescInfos[pair.first] = imageInfo;
		}
	
	
		for (auto& pair : buffers)
		{
			bufferDescInfos[pair.first] = pair.second.GetDescriptorInfo();
		}
		for (auto& pair : storageBuffers)
		{
			bufferDescInfos[pair.first] = pair.second.GetDescriptorInfo();
		}
	}
	
	ShaderPtr Material::InitShader(const std::string& inResourcePath)
	{
		if (!inResourcePath.empty())
		{
			ShaderPtr shader = DataManager::RequestResourceType<Shader>(inResourcePath);
			ProcessDescriptorType<Texture2DPtr>(DescriptorType::eSampledImage, shader, sampledImages2D, descriptorBindings);
			ProcessDescriptorType<Texture2DPtr>(DescriptorType::eStorageImage, shader, storageImages2D, descriptorBindings);
			ProcessDescriptorType<VulkanBuffer>(DescriptorType::eUniformBuffer, shader, buffers, descriptorBindings);
			ProcessDescriptorType<VulkanBuffer>(DescriptorType::eStorageBuffer, shader, storageBuffers, descriptorBindings);
			return shader;
		}
		return ShaderPtr();
	}
	
	void Material::PrepareDescriptorWrites(ShaderPtr inShader)
	{
		if (inShader)
		{
			PrepareDescriptorWrites<DescriptorImageInfo>(DescriptorType::eSampledImage, inShader, imageDescInfos, descriptorWrites);
			PrepareDescriptorWrites<DescriptorImageInfo>(DescriptorType::eStorageImage, inShader, imageDescInfos, descriptorWrites);
			PrepareDescriptorWrites<DescriptorBufferInfo>(DescriptorType::eUniformBuffer, inShader, bufferDescInfos, descriptorWrites);
			PrepareDescriptorWrites<DescriptorBufferInfo>(DescriptorType::eStorageBuffer, inShader, bufferDescInfos, descriptorWrites);
		}
	}
	
}
