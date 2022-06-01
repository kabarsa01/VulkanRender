#pragma once

#include "vulkan/vulkan.hpp"

#include "data/Resource.h"
#include "data/Texture2D.h"
#include "render/shader/Shader.h"
#include "render/objects/VulkanDescriptorSet.h"
#include "render/shader/ShaderResourceMapper.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::PipelineShaderStageCreateInfo;
	using VULKAN_HPP_NAMESPACE::WriteDescriptorSet;
	using VULKAN_HPP_NAMESPACE::DescriptorImageInfo;
	using VULKAN_HPP_NAMESPACE::DescriptorBufferInfo;
	
	class VulkanDevice;
	
	class Material : public Resource
	{
	public:
		Material(HashString inId);
		Material(HashString inId, const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath);
		virtual ~Material();
	
		void LoadResources();
		HashString GetHash();
		HashString GetShaderHash();
	
		std::vector<DescriptorSet> GetDescriptorSets();
		std::vector<vk::DescriptorSetLayout> GetDescriptorSetLayouts();
	
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
		void SetTextureArray(const std::string& inName, const std::vector<TextureDataPtr>& inTexture2D);
		void SetTextureArray(const std::string& inName, const std::vector<Texture2DPtr>& inTexture2D);
		void SetStorageTexture(const std::string& inName, Texture2DPtr inTexture2D);
		void SetStorageTextureArray(const std::string& inName, const std::vector<Texture2DPtr>& inTexture2D);
		template<typename T>
		void SetUniformBuffer(const std::string& inName, T& inUniformBuffer);
		template<typename T>
		void SetStorageBuffer(const std::string& inName, T& inStorageBuffer);
		void SetUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData);
		void SetStorageBuffer(const std::string& inName, uint64_t inSize, const char* inData);
		void SetUniformBufferArray(const std::string& inName, uint32_t arraySize, uint64_t dataSize, const char* inData);
		void SetStorageBufferArray(const std::string& inName, uint32_t arraySize, uint64_t dataSize, const char* inData);
		void SetUniformBufferExternal(const std::string& inName, BufferDataPtr inBuffer);
		void SetStorageBufferExternal(const std::string& inName, BufferDataPtr inBuffer);
		void SetAccelerationStructure(const std::string& inName, vk::AccelerationStructureKHR inAccelStruct);
		template<typename T>
		void UpdateUniformBuffer(const std::string& inName, T& inUniformBuffer);
		void UpdateUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData);
		void UpdateStorageBuffer(const std::string& inName, uint64_t inSize, const char* inData);
	
		BufferDataPtr GetUniformBuffer(const std::string& inName);
		BufferDataPtr GetStorageBuffer(const std::string& inName);
		TextureDataPtr GetSampledTexture(const std::string& inName);
		TextureDataPtr GetStorageTexture(const std::string& inName);
		template<typename ...Args>
		std::vector<TextureDataPtr> GetSampledTextures(Args&& ...names);
		template<typename ...Args>
		std::vector<TextureDataPtr> GetStorageTextures(Args&& ...names);
	
		inline ShaderPtr GetVertexShader() { return m_vertexShader; }
		inline ShaderPtr GetFragmentShader() { return m_fragmentShader; }
		inline ShaderPtr GetComputeShader() { return m_computeShader; }
		inline const std::string& GetVertexEntrypoint() const { return m_vertexEntrypoint; };
		inline const std::string& GetFragmentEntrypoint() const { return m_fragmentEntrypoint; };
		inline const std::string& GetComputeEntrypoint() const { return m_computeEntrypoint; };

		inline std::vector<TextureDataPtr> GetAllTextures() const;
		inline std::vector<BufferDataPtr> GetAllBuffers() const;
	
		bool Create() override;
	protected:
		std::string m_vertexShaderPath;
		std::string m_fragmentShaderPath;
		std::string m_computeShaderPath;
		std::string m_vertexEntrypoint;
		std::string m_fragmentEntrypoint;
		std::string m_computeEntrypoint;
		HashString m_hash;
		HashString m_shaderHash;
	
		ShaderPtr m_vertexShader;
		ShaderPtr m_fragmentShader;
		ShaderPtr m_computeShader;
		std::map<HashString, Texture2DPtr> m_sampledImages2D;
		std::map<HashString, std::vector<TextureDataPtr>> m_sampledImage2DArrays;
		std::map<HashString, Texture2DPtr> m_storageImages2D;
		std::map<HashString, std::vector<TextureDataPtr>> m_storageImage2DArrays;
		std::map<HashString, BufferDataPtr> m_buffers;
		std::map<HashString, std::vector<BufferDataPtr>> m_bufferArrays;
		std::map<HashString, BufferDataPtr> m_storageBuffers;
		std::map<HashString, std::vector<BufferDataPtr>> m_storageBufferArrays;
		std::map<HashString, vk::AccelerationStructureKHR> m_accelerationStructures;
		std::map<HashString, std::vector<vk::AccelerationStructureKHR>> m_accelerationStructureArrays;
	
		VulkanDevice* m_vulkanDevice;
		ShaderResourceMapper m_resourceMapper;
	
		bool Destroy() override;
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
	
	template<typename ...Args>
	std::vector<TextureDataPtr> Material::GetSampledTextures(Args&& ...names)
	{
		std::vector<TextureDataPtr> res;
		(res.push_back(m_sampledImages2D[names]), ...);
		return res;
	}

	template<typename ...Args>
	std::vector<TextureDataPtr> Material::GetStorageTextures(Args&& ...names)
	{
		std::vector<TextureDataPtr> res;
		(res.push_back(m_storageImages2D[names]), ...);
		return res;
	}

}
