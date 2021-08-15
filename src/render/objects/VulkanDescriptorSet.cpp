#include "VulkanDescriptorSet.h"
#include "core/Engine.h"
#include "VulkanDevice.h"
#include "../Renderer.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::DescriptorSetLayoutCreateInfo;

	VulkanDescriptorSet::VulkanDescriptorSet()
		: m_set(nullptr)
		, m_layout(nullptr)
	{
	}
	
	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
	}
	
	void VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice)
	{
		assert(!m_set && !m_layout);

		m_vulkanDevice = inVulkanDevice;
		CreateLayout();
	
		m_set = Engine::GetRendererInstance()->GetDescriptorPools().AllocateSet({m_layout}, m_pool)[0];
	}
	
	void VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice, std::vector<VulkanDescriptorSet*>& inSets)
	{
		Create(inVulkanDevice, static_cast<uint32_t>(inSets.size()), inSets.data());
	}
	
	void VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice, uint32_t inCount, VulkanDescriptorSet** inSets)
	{
		std::vector<DescriptorSetLayout> layouts;
		for (uint32_t index = 0; index < inCount; index++)
		{
			inSets[index]->m_vulkanDevice = inVulkanDevice;
			layouts.push_back(inSets[index]->CreateLayout());
		}
	
		DescriptorPool pool;
		std::vector<DescriptorSet> sets = Engine::GetRendererInstance()->GetDescriptorPools().AllocateSet({ layouts }, pool);
	
		for (uint32_t index = 0; index < inCount; index++)
		{
			inSets[index]->m_set = sets[index];
			inSets[index]->m_pool = pool;
		}
	}
	
	std::vector<VulkanDescriptorSet> VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice, const std::vector<ShaderPtr>& inShaders)
	{
		std::vector<VulkanDescriptorSet> result;

		uint32_t maxSet = 0;
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, BindingInfo>> sets;
		for (ShaderPtr shader : inShaders)
		{
			if (!shader)
			{
				continue;
			}
			for (auto& pair : shader->GetBindingsTypes())
			{
				for (BindingInfo& info : pair.second)
				{
					if (info.set > maxSet) maxSet = info.set;
					sets[info.set][info.binding] = info;
				}
			}
		}

		if (sets.size() == 0)
		{
			return result;
		}
		result.resize(maxSet + 1);

		for (auto& setPair : sets)
		{
			VulkanDescriptorSet& set = result[setPair.first];
			for (auto& bindingInfoPair : setPair.second)
			{
				vk::DescriptorSetLayoutBinding layoutBinding = bindingInfoPair.second.ToLayoutBinding();
				set.AddBinding(layoutBinding);
			}
			set.Create(&Engine::GetRendererInstance()->GetVulkanDevice());
		}

		return result;
	}

	VulkanDescriptorSet VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice, const std::vector<ShaderPtr>& inShaders, uint32_t inSetIndex)
	{
		VulkanDescriptorSet result;

		for (ShaderPtr shader : inShaders)
		{
			if (!shader)
			{
				continue;
			}
			for (auto& pair : shader->GetBindingsTypes())
			{
				for (BindingInfo& info : pair.second)
				{
					if (info.set == inSetIndex)
					{
						result.AddBinding(info.ToLayoutBinding());
					}
				}
			}
		}

		result.Create(&Engine::GetRendererInstance()->GetVulkanDevice());

		return result;
	}

	std::vector<VulkanDescriptorSet> VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice, const std::vector<RtShaderPtr>& inShaders)
	{
		std::vector<ShaderPtr> shaders;
		shaders.resize(inShaders.size());
		for (uint32_t idx = 0; idx < shaders.size(); idx++)
		{
			shaders[idx] = inShaders[idx];
		}
		return Create(inVulkanDevice, shaders);
	}

	VulkanDescriptorSet VulkanDescriptorSet::Create(VulkanDevice* inVulkanDevice, const std::vector<RtShaderPtr>& inShaders, uint32_t inSetIndex)
	{
		std::vector<ShaderPtr> shaders;
		shaders.resize(inShaders.size());
		for (uint32_t idx = 0; idx < shaders.size(); idx++)
		{
			shaders[idx] = inShaders[idx];
		}
		return Create(inVulkanDevice, shaders, inSetIndex);
	}

	void VulkanDescriptorSet::Update(std::vector<vk::WriteDescriptorSet>& writes)
	{
		for (vk::WriteDescriptorSet& write : writes)
		{
			write.setDstSet(m_set);
		}

		m_vulkanDevice->GetDevice().updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
	}

	void VulkanDescriptorSet::Destroy()
	{
		if (m_layout)
		{
			m_vulkanDevice->GetDevice().destroyDescriptorSetLayout(m_layout);
			m_layout = nullptr;
		}
		if (m_set)
		{
			m_vulkanDevice->GetDevice().freeDescriptorSets(m_pool, { m_set });
			m_set = nullptr;
		}
	}
	
	void VulkanDescriptorSet::SetBindings(const std::vector<DescriptorSetLayoutBinding>& inBindings)
	{
		m_bindings = inBindings;
	}

	void VulkanDescriptorSet::AddBinding(const DescriptorSetLayoutBinding& inBinding)
	{
		m_bindings.push_back(inBinding);
	}

	std::vector<DescriptorSetLayoutBinding> VulkanDescriptorSet::ProduceCustomBindings()
	{
		return {};
	}
	
	DescriptorSetLayout& VulkanDescriptorSet::CreateLayout()
	{
		if (m_bindings.size() == 0)
		{
			m_bindings = ProduceCustomBindings();
		}

		if (m_layout)
		{
			return m_layout;
		}

		vk::StructureChain<vk::DescriptorSetLayoutCreateInfo, vk::DescriptorSetLayoutBindingFlagsCreateInfo> chain;
		auto& flagsStruct = chain.get<vk::DescriptorSetLayoutBindingFlagsCreateInfo>();

		// maybe use vk::DescriptorBindingFlagBits::eVariableDescriptorCount in the future
		std::vector<vk::DescriptorBindingFlags> flags(m_bindings.size(), vk::DescriptorBindingFlagBits::ePartiallyBound);
		flagsStruct.setBindingFlags(flags);
	
		vk::DescriptorSetLayoutCreateInfo& layoutInfo = chain.get<vk::DescriptorSetLayoutCreateInfo>();
		layoutInfo.setBindingCount(static_cast<uint32_t>(m_bindings.size()));
		layoutInfo.setPBindings(m_bindings.data());
		m_layout = m_vulkanDevice->GetDevice().createDescriptorSetLayout(layoutInfo);
	
		return m_layout;
	}
	
}
