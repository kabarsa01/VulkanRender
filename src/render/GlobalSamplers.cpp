#include "GlobalSamplers.h"

GlobalSamplers::GlobalSamplers()
{

}

GlobalSamplers::~GlobalSamplers()
{

}

void GlobalSamplers::Create(VulkanDevice* inVulkanDevice)
{
	SamplerCreateInfo samplerInfo;
	samplerInfo.setAddressModeU(SamplerAddressMode::eRepeat);
	samplerInfo.setAddressModeV(SamplerAddressMode::eRepeat);
	samplerInfo.setAddressModeW(SamplerAddressMode::eRepeat);
	samplerInfo.setAnisotropyEnable(VK_FALSE);
	samplerInfo.setBorderColor(BorderColor::eIntOpaqueBlack);
	samplerInfo.setCompareEnable(VK_FALSE);
	samplerInfo.setMagFilter(Filter::eLinear);
	samplerInfo.setMaxAnisotropy(2);
	samplerInfo.setMaxLod(0);
	samplerInfo.setMinFilter(Filter::eLinear);
	samplerInfo.setMinLod(0);
	samplerInfo.setMipLodBias(0.0f);
	samplerInfo.setMipmapMode(SamplerMipmapMode::eLinear);
	samplerInfo.setUnnormalizedCoordinates(VK_FALSE);
	// something like this
	repeatLinearMipLinear = vulkanDevice->GetDevice().createSampler(samplerInfo);
}

void GlobalSamplers::Destroy()
{
	vulkanDevice->GetDevice().destroySampler(repeatLinearMipLinear);
}

