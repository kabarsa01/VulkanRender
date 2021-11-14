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
		void SetUniformBufferExternal(const std::string& inName, const VulkanBuffer& inBuffer);
		void SetStorageBufferExternal(const std::string& inName, const VulkanBuffer& inBuffer);
		void SetAccelerationStructure(const std::string& inName, vk::AccelerationStructureKHR inAccelStruct);
		template<typename T>
		void UpdateUniformBuffer(const std::string& inName, T& inUniformBuffer);
		void UpdateUniformBuffer(const std::string& inName, uint64_t inSize, const char* inData);
		void UpdateStorageBuffer(const std::string& inName, uint64_t inSize, const char* inData);
	
		VulkanBuffer& GetUniformBuffer(const std::string& inName);
		VulkanBuffer& GetStorageBuffer(const std::string& inName);
	
		inline ShaderPtr GetVertexShader() { return vertexShader; }
		inline ShaderPtr GetFragmentShader() { return fragmentShader; }
		inline ShaderPtr GetComputeShader() { return computeShader; }
		inline const std::string& GetVertexEntrypoint() const { return vertexEntrypoint; };
		inline const std::string& GetFragmentEntrypoint() const { return fragmentEntrypoint; };
		inline const std::string& GetComputeEntrypoint() const { return computeEntrypoint; };
	
		bool Create() override;
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
		std::map<HashString, std::vector<TextureDataPtr>> sampledImage2DArrays;
		std::map<HashString, Texture2DPtr> storageImages2D;
		std::map<HashString, std::vector<TextureDataPtr>> storageImage2DArrays;
		std::map<HashString, VulkanBuffer> buffers;
		std::map<HashString, std::vector<VulkanBuffer>> bufferArrays;
		std::map<HashString, VulkanBuffer> storageBuffers;
		std::map<HashString, std::vector<VulkanBuffer>> storageBufferArrays;
		std::map<HashString, vk::AccelerationStructureKHR> accelerationStructures;
		std::map<HashString, std::vector<vk::AccelerationStructureKHR>> accelerationStructureArrays;
	
		VulkanDevice* vulkanDevice;
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
	
}
