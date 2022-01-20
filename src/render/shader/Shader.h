#pragma once

#include "core\ObjectBase.h"
#include <memory>
#include <string>
#include <vector>
#include "vulkan\vulkan.hpp"
#include "data\Resource.h"
#include "spirv_cross\spirv_cross.hpp"
#include "..\GlobalSamplers.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::DescriptorType;
	using VULKAN_HPP_NAMESPACE::ShaderModule;

	struct BindingInfo
	{
		int32_t set = -1;
		int32_t binding = -1;
		uint32_t vectorSize;
		uint32_t numColumns;
		DescriptorType descriptorType;
		HashString name;
		HashString blockName;
		std::vector<uint32_t> arrayDimensions;

		bool isValid()
		{
			return set >= 0 && binding >= 0;
		}
	
		bool IsArray()
		{
			return arrayDimensions.size() > 0;
		}

		vk::DescriptorSetLayoutBinding ToLayoutBinding()
		{
			vk::DescriptorSetLayoutBinding layoutBinding;
			layoutBinding.setBinding(binding);
			layoutBinding.setDescriptorType(descriptorType);
			layoutBinding.setDescriptorCount(IsArray() ? arrayDimensions[0] : 1);
			layoutBinding.setStageFlags(vk::ShaderStageFlagBits::eAll);

			if (descriptorType == vk::DescriptorType::eSampler)
			{
				vk::Sampler* sampler = GlobalSamplers::GetInstance()->GetSampler(name);
				if (sampler)
				{
					layoutBinding.setPImmutableSamplers(sampler);
				}
			}
				
			return layoutBinding;
		}
	};
	
	class Shader : public Resource
	{
	public:
		Shader(const HashString& inPath);
		virtual ~Shader();
	
		virtual bool Create() override;
		virtual bool Destroy() override;
	
		ShaderModule GetShaderModule();
		void DestroyShaderModule();
		const std::vector<char>& GetCode() const;
	
		std::vector<BindingInfo>& GetAllBindings();
		std::vector<BindingInfo>& GetBindingsTypes(DescriptorType inDescriptorType);
		bool HasBinding(HashString name);
		BindingInfo GetBindingSafe(HashString name);
		BindingInfo& GetBinding(HashString name);
		std::unordered_map<HashString, BindingInfo>& GetBindingsNames() { return m_bindingsNames; }
		std::unordered_map<DescriptorType, std::vector<BindingInfo>>& GetBindingsTypes() { return m_bindingsTypes; }
	protected:
		std::string m_filePath;
		std::vector<char> m_binary;
		std::vector<BindingInfo> m_bindings;
		std::unordered_map<DescriptorType, std::vector<BindingInfo>> m_bindingsTypes;
		std::unordered_map<HashString, BindingInfo> m_bindingsNames;
	
		ShaderModule m_shaderModule;
	
		void CreateShaderModule();
		void ExtractBindingInfo(
			SPIRV_CROSS_NAMESPACE::SmallVector<SPIRV_CROSS_NAMESPACE::Resource>& inResources, 
			SPIRV_CROSS_NAMESPACE::Compiler& inCompiler,
			DescriptorType inDescriptorType);
		void ExtractBindingsInfo();
	};
	
	typedef std::shared_ptr<Shader> ShaderPtr;
}
