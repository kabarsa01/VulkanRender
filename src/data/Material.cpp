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
		, m_vertexEntrypoint("main")
		, m_fragmentEntrypoint("main")
		, m_computeEntrypoint("main")
	{
	
	}
	
	Material::Material(HashString inId, const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath)
		: Resource(inId)
		, m_vertexShaderPath(inVertexShaderPath)
		, m_fragmentShaderPath(inFragmentShaderPath)
		, m_vertexEntrypoint("main")
		, m_fragmentEntrypoint("main")
		, m_computeEntrypoint("main")
	{
	
	}
	
	Material::~Material()
	{
	}
	
	void Material::LoadResources()
	{
		// process all the material resources	
		m_vertexShader = DataManager::RequestResourceType<Shader>(m_vertexShaderPath);
		m_fragmentShader = DataManager::RequestResourceType<Shader>(m_fragmentShaderPath);
		m_computeShader = DataManager::RequestResourceType<Shader>(m_computeShaderPath);
	
		m_shaderHash = HashString(m_vertexShaderPath + m_fragmentShaderPath + m_computeShaderPath);

		m_resourceMapper.SetShaders({ m_vertexShader, m_fragmentShader, m_computeShader });
		for (auto& pair : m_sampledImages2D)
		{
			m_resourceMapper.AddSampledImage(pair.first, pair.second);
		}
		for (auto& pair : m_sampledImage2DArrays)
		{
			m_resourceMapper.AddSampledImageArray(pair.first, pair.second);
		}
		for (auto& pair : m_storageImages2D)
		{
			m_resourceMapper.AddStorageImage(pair.first, pair.second);
		}
		for (auto& pair : m_storageImage2DArrays)
		{
			m_resourceMapper.AddStorageImageArray(pair.first, pair.second);
		}
		for (auto& pair : m_buffers)
		{
			m_resourceMapper.AddUniformBuffer(pair.first, pair.second);
		}
		for (auto& pair : m_bufferArrays)
		{
			m_resourceMapper.AddUniformBufferArray(pair.first, pair.second);
		}
		for (auto& pair : m_storageBuffers)
		{
			m_resourceMapper.AddStorageBuffer(pair.first, pair.second);
		}
		for (auto& pair : m_storageBufferArrays)
		{
			m_resourceMapper.AddStorageBufferArray(pair.first, pair.second);
		}
		for (auto& pair : m_accelerationStructures)
		{
			m_resourceMapper.AddAccelerationStructure(pair.first, pair.second);
		}
		for (auto& pair : m_accelerationStructureArrays)
		{
			m_resourceMapper.AddAccelerationStructureArray(pair.first, pair.second);
		}
		m_resourceMapper.Update();
	}
	
	HashString Material::GetShaderHash()
	{
		return m_shaderHash;
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
		vertStageInfo.setModule(m_vertexShader->GetShaderModule());
		vertStageInfo.setPName(m_vertexEntrypoint.c_str());
	
		return vertStageInfo;
	}
	
	PipelineShaderStageCreateInfo Material::GetFragmentStageInfo()
	{
		PipelineShaderStageCreateInfo fragStageInfo;
		fragStageInfo.setStage(ShaderStageFlagBits::eFragment);
		fragStageInfo.setModule(m_fragmentShader->GetShaderModule());
		fragStageInfo.setPName(m_fragmentEntrypoint.c_str());
	
		return fragStageInfo;
	}
	
	PipelineShaderStageCreateInfo Material::GetComputeStageInfo()
	{
		PipelineShaderStageCreateInfo computeStageInfo;
		computeStageInfo.setStage(ShaderStageFlagBits::eCompute);
		computeStageInfo.setModule(m_computeShader->GetShaderModule());
		computeStageInfo.setPName(m_computeEntrypoint.c_str());
	
		return computeStageInfo;
	}
	
	void Material::SetEntrypoints(const std::string& inVertexEntrypoint, const std::string& inFragmentEntrypoint)
	{
		m_vertexEntrypoint = inVertexEntrypoint;
		m_fragmentEntrypoint = inFragmentEntrypoint;
	}
	
	void Material::SetVertexEntrypoint(const std::string& inEntrypoint)
	{
		m_vertexEntrypoint = inEntrypoint;
	}
	
	void Material::SetFragmentEntrypoint(const std::string& inEntrypoint)
	{
		m_fragmentEntrypoint = inEntrypoint;
	}
	
	void Material::SetComputeEntrypoint(const std::string& inEntrypoint)
	{
		m_computeEntrypoint = inEntrypoint;
	}
	
	void Material::SetShaderPath(const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath)
	{
		m_vertexShaderPath = inVertexShaderPath;
		m_fragmentShaderPath = inFragmentShaderPath;
	}
	
	void Material::SetComputeShaderPath(const std::string& inComputeShaderPath)
	{
		m_computeShaderPath = inComputeShaderPath;
	}
	
	void Material::SetTexture(const std::string& inName, Texture2DPtr inTexture2D)
	{
		m_sampledImages2D[inName] = inTexture2D;
	}

	void Material::SetTextureArray(const std::string& inName, const std::vector<TextureDataPtr>& inTexture2D)
	{
		m_sampledImage2DArrays[inName] = inTexture2D;
	}
	
	void Material::SetTextureArray(const std::string& inName, const std::vector<Texture2DPtr>& inTexture2D)
	{
		std::vector<TextureDataPtr>& textures = m_sampledImage2DArrays[inName];
		textures.resize(inTexture2D.size());
		for (uint32_t idx = 0; idx < inTexture2D.size(); ++idx)
		{
			textures[idx] = ObjectBase::Cast<TextureData>(inTexture2D[idx]);
		}
	}

	void Material::SetStorageTexture(const std::string& inName, Texture2DPtr inTexture2D)
	{
		m_storageImages2D[inName] = inTexture2D;
	}
	
	void Material::SetStorageTextureArray(const std::string& inName, const std::vector<Texture2DPtr>& inTexture2D)
	{
		std::vector<TextureDataPtr>& textures = m_storageImage2DArrays[inName];
		textures.resize(inTexture2D.size());
		for (uint32_t idx = 0; idx < inTexture2D.size(); ++idx)
		{
			textures[idx] = ObjectBase::Cast<TextureData>(inTexture2D[idx]);
		}
	}

	void Material::SetUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData)
	{
		VulkanDevice& vulkanDevice = Engine::GetRendererInstance()->GetVulkanDevice();
	
		m_buffers[inName] = nullptr;
		m_buffers[inName] = ResourceUtils::CreateBufferData(
			GetResourceId() + inName, 
			inSize, 
			BufferUsageFlagBits::eUniformBuffer | BufferUsageFlagBits::eTransferDst, 
			true);
	
		UpdateUniformBuffer(inName, inSize, inData);
	}
	
	void Material::SetUniformBufferExternal(const std::string& inName, BufferDataPtr inBuffer)
	{
		m_buffers[inName] = inBuffer;
	}
	
	void Material::SetStorageBufferExternal(const std::string& inName, BufferDataPtr inBuffer)
	{
		m_storageBuffers[inName] = inBuffer;
	}
	
	void Material::SetAccelerationStructure(const std::string& inName, vk::AccelerationStructureKHR inAccelStruct)
	{
		m_accelerationStructures[inName] = inAccelStruct;
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

		m_storageBuffers[inName] = buffer;
	}
	
	void Material::SetUniformBufferArray(const std::string& inName, uint32_t arraySize, uint64_t dataSize, const char* inData)
	{
		auto newArray = ResourceUtils::CreateBufferDataArray(GetResourceId() + inName, arraySize, dataSize, vk::BufferUsageFlagBits::eUniformBuffer, true);
		m_bufferArrays[inName] = newArray;
		if (inData)
		{
			for (uint32_t idx = 0; idx < arraySize; ++idx)
			{
				newArray[idx]->CopyTo(dataSize, inData);
			}
		}
	}

	void Material::SetStorageBufferArray(const std::string& inName, uint32_t arraySize, uint64_t dataSize, const char* inData)
	{
		auto newArray = ResourceUtils::CreateBufferDataArray(GetResourceId() + inName, arraySize, dataSize, vk::BufferUsageFlagBits::eStorageBuffer, true);
		m_storageBufferArrays[inName] = newArray;
		if (inData)
		{
			for (uint32_t idx = 0; idx < arraySize; ++idx)
			{
				newArray[idx]->CopyTo(dataSize, inData);
			}
		}
	}

	void Material::UpdateUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData)
	{
		m_buffers[inName]->CopyTo(inSize, inData);
	}
	
	void Material::UpdateStorageBuffer(const std::string& inName, uint64_t inSize, const char* inData)
	{
		m_storageBuffers[inName]->CopyTo(inSize, inData);
	}
	
	BufferDataPtr Material::GetUniformBuffer(const std::string& inName)
	{
		return m_buffers[inName];
	}
	
	BufferDataPtr Material::GetStorageBuffer(const std::string& inName)
	{
		return m_storageBuffers[inName];
	}
	
	TextureDataPtr Material::GetSampledTexture(const std::string& inName)
	{
		return m_sampledImages2D[inName];
	}

	TextureDataPtr Material::GetStorageTexture(const std::string& inName)
	{
		return m_storageImages2D[inName];
	}

	std::vector<TextureDataPtr> Material::GetAllTextures() const
	{
		std::vector<TextureDataPtr> result;

		for (auto& pair : m_sampledImages2D) 
		{
			result.push_back(pair.second);
		}
		for (auto& pair : m_storageImages2D)
		{
			result.push_back(pair.second);
		}
		for (auto& pair : m_sampledImage2DArrays)
		{
			for (auto texture : pair.second)
			{
				result.push_back(texture);
			}
		}
		for (auto& pair : m_storageImage2DArrays)
		{
			for (auto texture : pair.second)
			{
				result.push_back(texture);
			}
		}

		return result;
	}

	std::vector<BufferDataPtr> Material::GetAllBuffers() const
	{
		std::vector<BufferDataPtr> result;

		for (auto& pair : m_buffers)
		{
			result.push_back(pair.second);
		}
		for (auto& pair : m_storageBuffers)
		{
			result.push_back(pair.second);
		}
		for (auto& pair : m_bufferArrays)
		{
			for (auto buffer : pair.second)
			{
				result.push_back(buffer);
			}
		}
		for (auto& pair : m_storageBufferArrays)
		{
			for (auto buffer : pair.second)
			{
				result.push_back(buffer);
			}
		}

		return result;
	}

	bool Material::Create()
	{
		return true;
	}
	
	bool Material::Destroy()
	{
		// cleanup buffers. textures are resources themselves and will be cleaned by data manager
		for (auto& pair : m_buffers)
		{
//			pair.second.Destroy();
		}
		for (auto& pair : m_storageBuffers)
		{
//			pair.second.Destroy();
		}
		m_resourceMapper.Destroy();
		return true;
	}
	
}
