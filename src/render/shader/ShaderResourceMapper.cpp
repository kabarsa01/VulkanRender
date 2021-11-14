#include "ShaderResourceMapper.h"
#include "core/Engine.h"
#include "../Renderer.h"
#include "../objects/VulkanDevice.h"
#include "utils/ResourceUtils.h"
#include "data/Texture2D.h"

namespace CGE
{

	void ShaderResourceMapper::SetShaders(const std::vector<RtShaderPtr>& shaders)
	{
		std::vector<ShaderPtr> shadersCast;
		for (RtShaderPtr rtShader : shaders)
		{
			shadersCast.push_back(ObjectBase::Cast<Shader>(rtShader));
		}
		SetShaders(shadersCast);
	}

	void ShaderResourceMapper::AddSampledImage(HashString name, TextureDataPtr texture)
	{
		AddResource<TextureDataPtr>(vk::DescriptorType::eSampledImage, name, { texture });
	}

	void ShaderResourceMapper::AddSampledImage(uint32_t set, uint32_t binding, TextureDataPtr texture)
	{
		AddResource<TextureDataPtr>(vk::DescriptorType::eSampledImage, set, binding, { texture });
	}

	void ShaderResourceMapper::AddSampledImageArray(HashString name, const std::vector<TextureDataPtr>& textures)
	{
		AddResource<TextureDataPtr>(vk::DescriptorType::eSampledImage, name, textures);
	}

	void ShaderResourceMapper::AddStorageImage(HashString name, TextureDataPtr texture)
	{
		AddResource<TextureDataPtr>(vk::DescriptorType::eStorageImage, name, { texture });
	}

	void ShaderResourceMapper::AddStorageImage(uint32_t set, uint32_t binding, TextureDataPtr texture)
	{
		AddResource<TextureDataPtr>(vk::DescriptorType::eStorageImage, set, binding, { texture });
	}

	void ShaderResourceMapper::AddStorageImageArray(HashString name, const std::vector<TextureDataPtr>& textures)
	{
		AddResource<TextureDataPtr>(vk::DescriptorType::eStorageImage, name, textures);
	}

	void ShaderResourceMapper::AddUniformBuffer(HashString name, VulkanBuffer buffer)
	{
		AddResource<VulkanBuffer>(vk::DescriptorType::eUniformBuffer, name, { buffer });
	}

	void ShaderResourceMapper::AddUniformBuffer(uint32_t set, uint32_t binding, VulkanBuffer buffer)
	{
		AddResource<VulkanBuffer>(vk::DescriptorType::eUniformBuffer, set, binding, { buffer });
	}

	void ShaderResourceMapper::AddUniformBufferArray(HashString name, const std::vector<VulkanBuffer>& buffers)
	{
		AddResource<VulkanBuffer>(vk::DescriptorType::eUniformBuffer, name, buffers);
	}

	void ShaderResourceMapper::AddStorageBuffer(HashString name, VulkanBuffer buffer)
	{
		AddResource<VulkanBuffer>(vk::DescriptorType::eStorageBuffer, name, { buffer });
	}

	void ShaderResourceMapper::AddStorageBuffer(uint32_t set, uint32_t binding, VulkanBuffer buffer)
	{
		AddResource<VulkanBuffer>(vk::DescriptorType::eStorageBuffer, set, binding, { buffer });
	}

	void ShaderResourceMapper::AddStorageBufferArray(HashString name, const std::vector<VulkanBuffer>& buffers)
	{
		AddResource<VulkanBuffer>(vk::DescriptorType::eStorageBuffer, name, buffers);
	}

	void ShaderResourceMapper::AddAccelerationStructure(HashString name, vk::AccelerationStructureKHR accelerationStructure)
	{
		AddResource<vk::AccelerationStructureKHR>(vk::DescriptorType::eAccelerationStructureKHR, name, { accelerationStructure });
	}

	void ShaderResourceMapper::AddAccelerationStructure(uint32_t set, uint32_t binding, vk::AccelerationStructureKHR accelerationStructure)
	{
		AddResource<vk::AccelerationStructureKHR>(vk::DescriptorType::eAccelerationStructureKHR, set, binding, { accelerationStructure });
	}

	void ShaderResourceMapper::AddAccelerationStructureArray(HashString name, const std::vector<vk::AccelerationStructureKHR>& accelerationStructures)
	{
		AddResource<vk::AccelerationStructureKHR>(vk::DescriptorType::eAccelerationStructureKHR, name, accelerationStructures);
	}

	void ShaderResourceMapper::Update()
	{
		VulkanDevice* device = &Engine::GetRendererInstance()->GetVulkanDevice();

		for (VulkanDescriptorSet& set : m_sets)
		{
			set.Destroy();
		}
		m_sets.clear();
		// collect all data
		m_sets = VulkanDescriptorSet::Create(device, m_shaders);
		m_bindingsNames.clear();
		for (ShaderPtr shader : m_shaders)
		{
			if (!shader)
			{
				continue;
			}
			for (BindingInfo& bindingInfo : shader->GetAllBindings())
			{
				m_bindingsNames[bindingInfo.name] = bindingInfo;
			}
		}

		// remap names to sets and bindings
		for (auto& pair : m_resourcesNames)
		{
			if (m_bindingsNames.find(pair.first) != m_bindingsNames.end())
			{
				BindingInfo info = m_bindingsNames[pair.first];
				m_resourcesTable[info.descriptorType].push_back({ pair.second, info.set, info.binding });
			}
		}

		// make writes
		std::vector<vk::WriteDescriptorSet> writes;
		for (auto& pair : m_resourcesTable)
		{
			for (ResourceBindingRecord& rec : pair.second)
			{
				vk::WriteDescriptorSet write;
				switch (pair.first)
				{
				case vk::DescriptorType::eSampledImage:
					std::vector<vk::DescriptorImageInfo>* sampledImageInfo;
					sampledImageInfo = new std::vector<vk::DescriptorImageInfo>();
					m_writeInfos[pair.first].push_back(reinterpret_cast<char*>(sampledImageInfo));
					write = ResourceUtils::CreateWriteDescriptor(
						std::any_cast<std::vector<TextureDataPtr>&>(rec.resources),
						pair.first,
						rec.binding,
						vk::ImageLayout::eShaderReadOnlyOptimal,
						*sampledImageInfo
					);
					break;
				case vk::DescriptorType::eStorageImage:
					std::vector<vk::DescriptorImageInfo>* storageImageInfo;
					storageImageInfo = new std::vector<vk::DescriptorImageInfo>();
					m_writeInfos[pair.first].push_back(reinterpret_cast<char*>(storageImageInfo));
					write = ResourceUtils::CreateWriteDescriptor(
						std::any_cast<std::vector<TextureDataPtr>&>(rec.resources),
						pair.first,
						rec.binding,
						vk::ImageLayout::eGeneral,
						*storageImageInfo
					);
					break;
				case vk::DescriptorType::eUniformBuffer:
					std::vector<vk::DescriptorBufferInfo>* uniformBuffInfo;
					uniformBuffInfo = new std::vector<vk::DescriptorBufferInfo>();
					m_writeInfos[pair.first].push_back(reinterpret_cast<char*>(uniformBuffInfo));
					write = ResourceUtils::CreateWriteDescriptor(
						std::any_cast<std::vector<VulkanBuffer>&>(rec.resources),
						pair.first,
						rec.binding,
						*uniformBuffInfo
					);
					break;
				case vk::DescriptorType::eStorageBuffer:
					std::vector<vk::DescriptorBufferInfo>* storageBuffInfo;
					storageBuffInfo = new std::vector<vk::DescriptorBufferInfo>();
					m_writeInfos[pair.first].push_back(reinterpret_cast<char*>(storageBuffInfo));
					write = ResourceUtils::CreateWriteDescriptor(
						std::any_cast<std::vector<VulkanBuffer>&>(rec.resources),
						pair.first,
						rec.binding,
						*storageBuffInfo
					);
					break;
				case vk::DescriptorType::eAccelerationStructureKHR:
					vk::WriteDescriptorSetAccelerationStructureKHR* accelStructWrite;
					accelStructWrite = new vk::WriteDescriptorSetAccelerationStructureKHR();
					m_writeInfos[pair.first].push_back(reinterpret_cast<char*>(accelStructWrite));
					write = ResourceUtils::CreateWriteDescriptor(
						std::any_cast<std::vector<vk::AccelerationStructureKHR>&>(rec.resources),
						pair.first,
						rec.binding,
						*accelStructWrite
					);
					break;
				}
				write.setDstSet(m_sets[rec.set].GetSet());
				writes.push_back(write);
			}
		}

		device->GetDevice().updateDescriptorSets(static_cast<uint32_t>( writes.size() ), writes.data(), 0, nullptr);

		for (auto& pair : m_writeInfos)
		{
			for (char* info : pair.second)
			{
				switch (pair.first)
				{
				case vk::DescriptorType::eSampledImage:
				case vk::DescriptorType::eStorageImage:
					std::vector<vk::DescriptorImageInfo>* sampledImageInfo;
					sampledImageInfo = reinterpret_cast<std::vector<vk::DescriptorImageInfo>*>(info);
					delete sampledImageInfo;
					break;
				case vk::DescriptorType::eUniformBuffer:
				case vk::DescriptorType::eStorageBuffer:
					std::vector<vk::DescriptorBufferInfo>* bufferInfo;
					bufferInfo = reinterpret_cast<std::vector<vk::DescriptorBufferInfo>*>(info);
					delete bufferInfo;
					break;
				case vk::DescriptorType::eAccelerationStructureKHR:
					vk::WriteDescriptorSetAccelerationStructureKHR* accelStructInfo;
					accelStructInfo = reinterpret_cast<vk::WriteDescriptorSetAccelerationStructureKHR*>(info);
					delete accelStructInfo;
				}
			}
		}
		m_writeInfos.clear();
	}

	void ShaderResourceMapper::Destroy()
	{
		for (auto& set : m_sets)
		{
			set.Destroy();
		}
		m_sets.clear();
	}

}



