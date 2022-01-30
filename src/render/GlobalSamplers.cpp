#include "GlobalSamplers.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::SamplerCreateInfo;
	using VULKAN_HPP_NAMESPACE::SamplerAddressMode;
	using VULKAN_HPP_NAMESPACE::BorderColor;
	using VULKAN_HPP_NAMESPACE::Filter;
	using VULKAN_HPP_NAMESPACE::SamplerMipmapMode;
	using VULKAN_HPP_NAMESPACE::DescriptorType;
	using VULKAN_HPP_NAMESPACE::ShaderStageFlagBits;

	GlobalSamplers* GlobalSamplers::instance = new GlobalSamplers();
	
	GlobalSamplers::GlobalSamplers()
		: mipLodBias(0.0f)
	{
	}
	
	GlobalSamplers::GlobalSamplers(const GlobalSamplers& inOther)
	{
	}
	
	GlobalSamplers::~GlobalSamplers()
	{
	}
	
	void GlobalSamplers::operator=(const GlobalSamplers& inOther)
	{
	}
	
	void GlobalSamplers::Create(VulkanDevice* inVulkanDevice)
	{
		if (samplers.size() > 0)
		{
			return;
		}
	
		vulkanDevice = inVulkanDevice;
	
		SamplerCreateInfo samplerInfo;
		samplerInfo.setAddressModeU(SamplerAddressMode::eRepeat);
		samplerInfo.setAddressModeV(SamplerAddressMode::eRepeat);
		samplerInfo.setAddressModeW(SamplerAddressMode::eRepeat);
		samplerInfo.setAnisotropyEnable(VK_FALSE);
		samplerInfo.setBorderColor(BorderColor::eIntOpaqueBlack);
		samplerInfo.setCompareEnable(VK_TRUE);
		samplerInfo.setCompareOp(vk::CompareOp::eAlways);
		samplerInfo.setMagFilter(Filter::eLinear);
		samplerInfo.setMaxAnisotropy(2);
		samplerInfo.setMinLod(0.0f);
		samplerInfo.setMaxLod(12.0f);
		samplerInfo.setMipLodBias(mipLodBias);
		samplerInfo.setMinFilter(Filter::eLinear);
		samplerInfo.setMipmapMode(SamplerMipmapMode::eLinear);
		samplerInfo.setUnnormalizedCoordinates(VK_FALSE);
		repeatLinearMipLinear = vulkanDevice->GetDevice().createSampler(samplerInfo);
		samplers.push_back(&repeatLinearMipLinear);
		m_samplerNames["repeatLinearSampler"] = samplers.back();
	
		samplerInfo.setAddressModeU(SamplerAddressMode::eMirroredRepeat);
		samplerInfo.setAddressModeV(SamplerAddressMode::eMirroredRepeat);
		samplerInfo.setAddressModeW(SamplerAddressMode::eMirroredRepeat);
		repeatMirrorLinearMipLinear = vulkanDevice->GetDevice().createSampler(samplerInfo);
		samplers.push_back(&repeatMirrorLinearMipLinear);
		m_samplerNames["repeatMirrorLinearSampler"] = samplers.back();
	
		samplerInfo.setAddressModeU(SamplerAddressMode::eClampToBorder);
		samplerInfo.setAddressModeV(SamplerAddressMode::eClampToBorder);
		samplerInfo.setAddressModeW(SamplerAddressMode::eClampToBorder);
		borderBlackLinearMipLinear = vulkanDevice->GetDevice().createSampler(samplerInfo);
		samplers.push_back(&borderBlackLinearMipLinear);
		m_samplerNames["borderBlackLinearSampler"] = samplers.back();
	
		samplerInfo.setBorderColor(BorderColor::eIntOpaqueWhite);
		borderWhiteLinearMipLinear = vulkanDevice->GetDevice().createSampler(samplerInfo);
		samplers.push_back(&borderWhiteLinearMipLinear);
		m_samplerNames["borderWhiteLinearSampler"] = samplers.back();

		samplerInfo.setMagFilter(Filter::eNearest);
		samplerInfo.setMinFilter(Filter::eNearest);
		samplerInfo.setMipmapMode(SamplerMipmapMode::eNearest);
		samplerInfo.setBorderColor(BorderColor::eIntOpaqueBlack);
		borderBlackNearesMipNearest = vulkanDevice->GetDevice().createSampler(samplerInfo);
		samplers.push_back(&borderBlackNearesMipNearest);
		m_samplerNames["borderBlackNearestSampler"] = samplers.back();
	
		// construct bindings 
		ConstructBindings();
	}
	
	void GlobalSamplers::Destroy()
	{
		for (uint32_t index = 0; index < samplers.size(); index++)
		{
			vulkanDevice->GetDevice().destroySampler(*samplers[index]);
		}
		samplers.clear();
	}
	
	std::vector<DescriptorSetLayoutBinding> GlobalSamplers::GetBindings(uint32_t inStartIndex /*= 0*/)
	{
		for (uint32_t index = 0; index < bindings.size(); index++)
		{
			bindings[index].setBinding(inStartIndex + index);
		}
		return bindings;
	}
	
	void GlobalSamplers::ConstructBindings()
	{
		bindings.clear();
		for (uint32_t index = 0; index < samplers.size(); index++)
		{
			DescriptorSetLayoutBinding binding;
			binding.setBinding(index);
			binding.setDescriptorType(DescriptorType::eSampler);
			binding.setDescriptorCount(1);
			binding.setStageFlags(ShaderStageFlagBits::eAll);
			binding.setPImmutableSamplers(samplers[index]);
	
			bindings.push_back(binding);
		}
	}
	
}
