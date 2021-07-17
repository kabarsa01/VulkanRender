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
		AddResource(vk::DescriptorType::eSampledImage, name, texture);
	}

	void ShaderResourceMapper::AddSampledImage(uint32_t set, uint32_t binding, TextureDataPtr texture)
	{
		AddResource(vk::DescriptorType::eSampledImage, set, binding, texture);
	}

	void ShaderResourceMapper::AddStorageImage(HashString name, TextureDataPtr texture)
	{
		AddResource(vk::DescriptorType::eStorageImage, name, texture);
	}

	void ShaderResourceMapper::AddStorageImage(uint32_t set, uint32_t binding, TextureDataPtr texture)
	{
		AddResource(vk::DescriptorType::eStorageImage, set, binding, texture);
	}

	void ShaderResourceMapper::AddUniformBuffer(HashString name, VulkanBuffer buffer)
	{
		AddResource(vk::DescriptorType::eUniformBuffer, name, buffer);
	}

	void ShaderResourceMapper::AddUniformBuffer(uint32_t set, uint32_t binding, VulkanBuffer buffer)
	{
		AddResource(vk::DescriptorType::eUniformBuffer, set, binding, buffer);
	}

	void ShaderResourceMapper::AddStorageBuffer(HashString name, VulkanBuffer buffer)
	{
		AddResource(vk::DescriptorType::eStorageBuffer, name, buffer);
	}

	void ShaderResourceMapper::AddStorageBuffer(uint32_t set, uint32_t binding, VulkanBuffer buffer)
	{
		AddResource(vk::DescriptorType::eStorageBuffer, set, binding, buffer);
	}

	void ShaderResourceMapper::AddAccelerationStructure(HashString name, vk::AccelerationStructureKHR accelerationStructure)
	{
		AddResource(vk::DescriptorType::eAccelerationStructureKHR, name, accelerationStructure);
	}

	void ShaderResourceMapper::AddAccelerationStructure(uint32_t set, uint32_t binding, vk::AccelerationStructureKHR accelerationStructure)
	{
		AddResource(vk::DescriptorType::eAccelerationStructureKHR, set, binding, accelerationStructure);
	}

	void ShaderResourceMapper::Update()
	{
		VulkanDevice* device = &Engine::GetRendererInstance()->GetVulkanDevice();

		for (VulkanDescriptorSet& set : m_sets)
		{
			set.Destroy();
		}
		// collect all data
		m_sets = VulkanDescriptorSet::Create(device, m_shaders);
		m_bindingsNames.clear();
		for (ShaderPtr shader : m_shaders)
		{
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
					vk::DescriptorImageInfo* sampledImageInfo;
					sampledImageInfo = new vk::DescriptorImageInfo();
					m_writeInfos[pair.first].push_back(reinterpret_cast<char*>(sampledImageInfo));
					write = ResourceUtils::CreateWriteDescriptor(
						std::any_cast<TextureDataPtr>(rec.resource),
						pair.first,
						rec.binding,
						vk::ImageLayout::eShaderReadOnlyOptimal,
						*sampledImageInfo
					);
					break;
				case vk::DescriptorType::eStorageImage:
					vk::DescriptorImageInfo* storageImageInfo;
					storageImageInfo = new vk::DescriptorImageInfo();
					m_writeInfos[pair.first].push_back(reinterpret_cast<char*>(storageImageInfo));
					write = ResourceUtils::CreateWriteDescriptor(
						std::any_cast<TextureDataPtr>(rec.resource),
						pair.first,
						rec.binding,
						vk::ImageLayout::eGeneral,
						*storageImageInfo
					);
					break;
				case vk::DescriptorType::eUniformBuffer:
					vk::DescriptorBufferInfo* uniformBuffInfo;
					uniformBuffInfo = new vk::DescriptorBufferInfo();
					m_writeInfos[pair.first].push_back(reinterpret_cast<char*>(uniformBuffInfo));
					write = ResourceUtils::CreateWriteDescriptor(
						std::any_cast<VulkanBuffer>(rec.resource),
						pair.first,
						rec.binding,
						*uniformBuffInfo
					);
					break;
				case vk::DescriptorType::eStorageBuffer:
					vk::DescriptorBufferInfo* storageBuffInfo;
					storageBuffInfo = new vk::DescriptorBufferInfo();
					m_writeInfos[pair.first].push_back(reinterpret_cast<char*>(storageBuffInfo));
					write = ResourceUtils::CreateWriteDescriptor(
						std::any_cast<VulkanBuffer>(rec.resource),
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
						std::any_cast<vk::AccelerationStructureKHR>(rec.resource),
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
					vk::DescriptorImageInfo* sampledImageInfo;
					sampledImageInfo = reinterpret_cast<vk::DescriptorImageInfo*>(info);
					delete sampledImageInfo;
					break;
				case vk::DescriptorType::eUniformBuffer:
				case vk::DescriptorType::eStorageBuffer:
					vk::DescriptorBufferInfo* bufferInfo;
					bufferInfo = reinterpret_cast<vk::DescriptorBufferInfo*>(info);
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

}



