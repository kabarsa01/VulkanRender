#pragma once

#include "vulkan/vulkan.hpp"
#include "objects/VulkanDevice.h"
#include <unordered_map>
#include "common/HashString.h"

namespace CGE
{
	
	using VULKAN_HPP_NAMESPACE::Sampler;
	using VULKAN_HPP_NAMESPACE::DescriptorSetLayoutBinding;
	
	// samplers to be used as immutable samplers
	class GlobalSamplers
	{
	public:
		Sampler repeatLinearMipLinear;
		Sampler repeatMirrorLinearMipLinear;
		Sampler borderBlackLinearMipLinear;
		Sampler borderWhiteLinearMipLinear;
	
		static GlobalSamplers* GetInstance() { return instance; }
	
		void Create(VulkanDevice* inVulkanDevice);
		void Destroy();
	
		inline void SetMipLodBias(float inMipLodBias) { mipLodBias = inMipLodBias; }
	
		Sampler* GetSampler(uint32_t index) { return samplers[index]; }
		Sampler* GetSampler(HashString name) { return m_samplerNames[name]; }
		std::vector<DescriptorSetLayoutBinding> GetBindings(uint32_t inStartIndex = 0);
	private:
		static GlobalSamplers* instance;
	
		VulkanDevice* vulkanDevice;
		std::vector<DescriptorSetLayoutBinding> bindings;
		std::vector<Sampler*> samplers;
		std::unordered_map<HashString, vk::Sampler*> m_samplerNames;
		float mipLodBias;
	
		GlobalSamplers();
		GlobalSamplers(const GlobalSamplers& inOther);
		virtual ~GlobalSamplers();
		void operator=(const GlobalSamplers& inOther);
	
		void ConstructBindings();
	};
}
