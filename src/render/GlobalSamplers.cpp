#include "GlobalSamplers.h"

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
	samplerInfo.setCompareEnable(VK_FALSE);
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

	samplerInfo.setAddressModeU(SamplerAddressMode::eMirroredRepeat);
	samplerInfo.setAddressModeV(SamplerAddressMode::eMirroredRepeat);
	samplerInfo.setAddressModeW(SamplerAddressMode::eMirroredRepeat);
	repeatMirrorLinearMipLinear = vulkanDevice->GetDevice().createSampler(samplerInfo);
	samplers.push_back(&repeatMirrorLinearMipLinear);

	samplerInfo.setAddressModeU(SamplerAddressMode::eClampToBorder);
	samplerInfo.setAddressModeV(SamplerAddressMode::eClampToBorder);
	samplerInfo.setAddressModeW(SamplerAddressMode::eClampToBorder);
	borderBlackLinearMipLinear = vulkanDevice->GetDevice().createSampler(samplerInfo);
	samplers.push_back(&borderBlackLinearMipLinear);

	samplerInfo.setBorderColor(BorderColor::eIntOpaqueWhite);
	borderWhiteLinearMipLinear = vulkanDevice->GetDevice().createSampler(samplerInfo);
	samplers.push_back(&borderWhiteLinearMipLinear);

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
		binding.setStageFlags(ShaderStageFlagBits::eAllGraphics | ShaderStageFlagBits::eCompute);
		binding.setPImmutableSamplers(samplers[index]);

		bindings.push_back(binding);
	}
}

