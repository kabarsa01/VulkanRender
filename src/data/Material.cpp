#include "Material.h"
#include "DataManager.h"
#include "core/Engine.h"
#include "render/Renderer.h"
#include "render/TransferList.h"
#include "utils/ResourceUtils.h"

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
		// process all the material resources	
		vertexShader = DataManager::RequestResourceType<Shader>(vertexShaderPath);
		fragmentShader = DataManager::RequestResourceType<Shader>(fragmentShaderPath);
		computeShader = DataManager::RequestResourceType<Shader>(computeShaderPath);
	
		shaderHash = HashString(vertexShaderPath + fragmentShaderPath + computeShaderPath);

		m_resourceMapper.SetShaders({ vertexShader, fragmentShader, computeShader });
		for (auto& pair : sampledImages2D)
		{
			m_resourceMapper.AddSampledImage(pair.first, pair.second);
		}
		for (auto& pair : sampledImage2DArrays)
		{
			m_resourceMapper.AddSampledImageArray(pair.first, pair.second);
		}
		for (auto& pair : storageImages2D)
		{
			m_resourceMapper.AddStorageImage(pair.first, pair.second);
		}
		for (auto& pair : storageImage2DArrays)
		{
			m_resourceMapper.AddStorageImageArray(pair.first, pair.second);
		}
		for (auto& pair : buffers)
		{
			m_resourceMapper.AddUniformBuffer(pair.first, pair.second);
		}
		for (auto& pair : bufferArrays)
		{
			m_resourceMapper.AddUniformBufferArray(pair.first, pair.second);
		}
		for (auto& pair : storageBuffers)
		{
			m_resourceMapper.AddStorageBuffer(pair.first, pair.second);
		}
		for (auto& pair : storageBufferArrays)
		{
			m_resourceMapper.AddStorageBufferArray(pair.first, pair.second);
		}
		for (auto& pair : accelerationStructures)
		{
			m_resourceMapper.AddAccelerationStructure(pair.first, pair.second);
		}
		for (auto& pair : accelerationStructureArrays)
		{
			m_resourceMapper.AddAccelerationStructureArray(pair.first, pair.second);
		}
		m_resourceMapper.Update();
	}
	
	HashString Material::GetShaderHash()
	{
		return shaderHash;
	}
	
	std::vector<DescriptorSet> Material::GetDescriptorSets()
	{
		std::vector<vk::DescriptorSet> sets;
		for (VulkanDescriptorSet& set : m_resourceMapper.GetDescriptorSets())
		{
			sets.push_back(set.GetSet());
		}
		return sets;
	}

	std::vector<vk::DescriptorSetLayout> Material::GetDescriptorSetLayouts()
	{
		std::vector<vk::DescriptorSetLayout> layouts;
		for (VulkanDescriptorSet& set : m_resourceMapper.GetDescriptorSets())
		{
			layouts.push_back(set.GetLayout());
		}
		return layouts;
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

	void Material::SetTextureArray(const std::string& inName, const std::vector<TextureDataPtr>& inTexture2D)
	{
		sampledImage2DArrays[inName] = inTexture2D;
	}
	
	void Material::SetTextureArray(const std::string& inName, const std::vector<Texture2DPtr>& inTexture2D)
	{
		std::vector<TextureDataPtr>& textures = sampledImage2DArrays[inName];
		textures.resize(inTexture2D.size());
		for (uint32_t idx = 0; idx < inTexture2D.size(); ++idx)
		{
			textures[idx] = ObjectBase::Cast<TextureData>(inTexture2D[idx]);
		}
	}

	void Material::SetStorageTexture(const std::string& inName, Texture2DPtr inTexture2D)
	{
		storageImages2D[inName] = inTexture2D;
	}
	
	void Material::SetStorageTextureArray(const std::string& inName, const std::vector<Texture2DPtr>& inTexture2D)
	{
		std::vector<TextureDataPtr>& textures = storageImage2DArrays[inName];
		textures.resize(inTexture2D.size());
		for (uint32_t idx = 0; idx < inTexture2D.size(); ++idx)
		{
			textures[idx] = ObjectBase::Cast<TextureData>(inTexture2D[idx]);
		}
	}

	void Material::SetUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData)
	{
		VulkanDevice& vulkanDevice = Engine::GetRendererInstance()->GetVulkanDevice();
	
		buffers[inName] = nullptr;
		buffers[inName] = ResourceUtils::CreateBufferData(
			GetResourceId() + inName, 
			inSize, 
			BufferUsageFlagBits::eUniformBuffer | BufferUsageFlagBits::eTransferDst, 
			true);
	
		UpdateUniformBuffer(inName, inSize, inData);
	}
	
	void Material::SetUniformBufferExternal(const std::string& inName, const BufferDataPtr& inBuffer)
	{
		buffers[inName] = nullptr;
		buffers[inName] = inBuffer;
//		buffers[inName].SetCleanup(false);
	}
	
	void Material::SetStorageBufferExternal(const std::string& inName, const BufferDataPtr& inBuffer)
	{
		storageBuffers[inName] = inBuffer;
//		storageBuffers[inName].SetCleanup(false);
	}
	
	void Material::SetAccelerationStructure(const std::string& inName, vk::AccelerationStructureKHR inAccelStruct)
	{
		accelerationStructures[inName] = inAccelStruct;
	}

	void Material::SetStorageBuffer(const std::string& inName, uint64_t inSize, const char* inData)
	{
		VulkanDevice& vulkanDevice = Engine::GetRendererInstance()->GetVulkanDevice();
	
		BufferDataPtr buffer = ResourceUtils::CreateBufferData(
			GetResourceId() + inName,
			inSize,
			BufferUsageFlagBits::eStorageBuffer | BufferUsageFlagBits::eTransferDst,
			true);
		buffer->CopyTo(inSize, inData);

		storageBuffers[inName] = buffer;
	}
	
	void Material::SetUniformBufferArray(const std::string& inName, uint32_t arraySize, uint64_t dataSize, const char* inData)
	{
		bufferArrays[inName].resize(arraySize);

		for (uint32_t idx = 0; idx < arraySize; ++idx)
		{

		}
	}

	void Material::SetStorageBufferArray(const std::string& inName, uint32_t arraySize, uint64_t dataSize, const char* inData)
	{

	}

	void Material::UpdateUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData)
	{
		buffers[inName]->CopyTo(inSize, inData);
	}
	
	void Material::UpdateStorageBuffer(const std::string& inName, uint64_t inSize, const char* inData)
	{
		storageBuffers[inName]->CopyTo(inSize, inData);
//		TransferList::GetInstance()->PushBuffer(&storageBuffers[inName]);
	}
	
	BufferDataPtr Material::GetUniformBuffer(const std::string& inName)
	{
		return buffers[inName];
	}
	
	BufferDataPtr Material::GetStorageBuffer(const std::string& inName)
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
//			pair.second.Destroy();
		}
		for (auto& pair : storageBuffers)
		{
//			pair.second.Destroy();
		}
		m_resourceMapper.Destroy();
		return true;
	}
	
}
