#ifndef __SHADER_RESOURCE_MAPPER_H__
#define __SHADER_RESOURCE_MAPPER_H__

#include <vector>
#include "Shader.h"
#include "../objects/VulkanDescriptorSet.h"
#include <any>
#include "data/TextureData.h"
#include "../resources/VulkanBuffer.h"
#include "RtShader.h"

namespace CGE
{

	class ShaderResourceMapper
	{
	public:
		ShaderResourceMapper() {}
		~ShaderResourceMapper()
		{
			for (VulkanDescriptorSet& set : m_sets)
			{
				set.Destroy();
			}
		}

		std::vector<VulkanDescriptorSet>& GetDescriptorSets() { return m_sets; }

		void SetShaders(const std::vector<ShaderPtr>& shaders) { m_shaders = shaders; }
		void SetShaders(const std::vector<RtShaderPtr>& shaders);
		void AddShader(ShaderPtr shader) { m_shaders.emplace_back(shader); }
		void AddShader(RtShaderPtr rtShader) { m_shaders.emplace_back(ObjectBase::Cast<Shader>(rtShader)); }

		void AddSampledImage(HashString name, TextureDataPtr texture);
		void AddSampledImage(uint32_t set, uint32_t binding, TextureDataPtr texture);
		void AddStorageImage(HashString name, TextureDataPtr texture);
		void AddStorageImage(uint32_t set, uint32_t binding, TextureDataPtr texture);
		void AddUniformBuffer(HashString name, VulkanBuffer buffer);
		void AddUniformBuffer(uint32_t set, uint32_t binding, VulkanBuffer buffer);
		void AddStorageBuffer(HashString name, VulkanBuffer buffer);
		void AddStorageBuffer(uint32_t set, uint32_t binding, VulkanBuffer buffer);
		void AddAccelerationStructure(HashString name, vk::AccelerationStructureKHR accelerationStructure);
		void AddAccelerationStructure(uint32_t set, uint32_t binding, vk::AccelerationStructureKHR accelerationStructure);

		void Update();
	private:
		struct ResourceBindingRecord
		{
			std::any resource;
			int32_t set;
			int32_t binding;
		};

		std::vector<ShaderPtr> m_shaders;
		std::vector<VulkanDescriptorSet> m_sets;

		std::unordered_map<HashString, std::any> m_resourcesNames;
		std::unordered_map<vk::DescriptorType, std::vector<char*>> m_writeInfos;
		std::unordered_map<vk::DescriptorType, std::vector<ResourceBindingRecord>> m_resourcesTable;

		std::unordered_map<HashString, BindingInfo> m_bindingsNames;

		template<typename T>
		void AddResource(vk::DescriptorType type, HashString name, T resource)
		{
			m_resourcesNames[name] = resource;
		}
		template<typename T>
		void AddResource(vk::DescriptorType type, uint32_t set, uint32_t binding, T resource)
		{
			m_resourcesTable[type].push_back({ resource, static_cast<int32_t>( set ), static_cast<int32_t>( binding ) });
		}
		template<typename T>
		T GetResource(HashString name)
		{
			return std::any_cast<T>(m_resourcesNames[name]);
		}
	};

}

#endif
