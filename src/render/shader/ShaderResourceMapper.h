#ifndef __SHADER_RESOURCE_MAPPER_H__
#define __SHADER_RESOURCE_MAPPER_H__

#include <vector>
#include "Shader.h"
#include "../objects/VulkanDescriptorSet.h"
#include <any>
#include "data/TextureData.h"
#include "RtShader.h"
#include "data/BufferData.h"

namespace CGE
{

	class ShaderResourceMapper
	{
	public:
		ShaderResourceMapper() {}
		~ShaderResourceMapper()
		{
			Destroy();
		}

		std::vector<VulkanDescriptorSet>& GetDescriptorSets() { return m_sets; }
		std::vector<vk::DescriptorSet>& GetNativeDescriptorSets() { return m_nativeSets; }
		std::vector<vk::DescriptorSetLayout> GetNativeLayouts();

		void SetShaders(const std::vector<ShaderPtr>& shaders) { m_shaders = shaders; }
		void SetShaders(const std::vector<RtShaderPtr>& shaders);
		void AddShader(ShaderPtr shader) { m_shaders.emplace_back(shader); }
		void AddShader(RtShaderPtr rtShader) { m_shaders.emplace_back(ObjectBase::Cast<Shader>(rtShader)); }

		void AddSampledImage(HashString name, TextureDataPtr texture);
		void AddSampledImage(uint32_t set, uint32_t binding, TextureDataPtr texture);
		void AddSampledImageArray(HashString name, const std::vector<TextureDataPtr>& textures);
		void AddStorageImage(HashString name, TextureDataPtr texture);
		void AddStorageImage(uint32_t set, uint32_t binding, TextureDataPtr texture);
		void AddStorageImageArray(HashString name, const std::vector<TextureDataPtr>& textures);
		void AddUniformBuffer(HashString name, BufferDataPtr buffer);
		void AddUniformBuffer(uint32_t set, uint32_t binding, BufferDataPtr buffer);
		void AddUniformBufferArray(HashString name, const std::vector<BufferDataPtr>& buffers);
		void AddStorageBuffer(HashString name, BufferDataPtr buffer);
		void AddStorageBuffer(uint32_t set, uint32_t binding, BufferDataPtr buffer);
		void AddStorageBufferArray(HashString name, const std::vector<BufferDataPtr>& buffers);
		void AddAccelerationStructure(HashString name, vk::AccelerationStructureKHR accelerationStructure);
		void AddAccelerationStructure(uint32_t set, uint32_t binding, vk::AccelerationStructureKHR accelerationStructure);
		void AddAccelerationStructureArray(HashString name, const std::vector<vk::AccelerationStructureKHR>& accelerationStructures);

		void Update();
		void Destroy();
	private:
		struct ResourceBindingRecord
		{
			std::any resources;
			int32_t set;
			int32_t binding;
		};

		std::vector<ShaderPtr> m_shaders;
		std::vector<VulkanDescriptorSet> m_sets;
		std::vector<vk::DescriptorSet> m_nativeSets;

		std::unordered_map<HashString, std::any> m_resourcesNames;
		std::unordered_map<vk::DescriptorType, std::vector<char*>> m_writeInfos;
		std::unordered_map<vk::DescriptorType, std::vector<ResourceBindingRecord>> m_resourcesTable;

		std::unordered_map<HashString, BindingInfo> m_bindingsNames;

		template<typename T>
		void AddResource(vk::DescriptorType type, HashString name, std::vector<T> resources)
		{
			m_resourcesNames[name] = resources;
		}
		template<typename T>
		void AddResource(vk::DescriptorType type, uint32_t set, uint32_t binding, std::vector<T> resources)
		{
			m_resourcesTable[type].push_back({ resources, static_cast<int32_t>( set ), static_cast<int32_t>( binding ) });
		}
		template<typename T>
		T GetResource(HashString name)
		{
			return std::any_cast<T>(m_resourcesNames[name]);
		}
	};

}

#endif
