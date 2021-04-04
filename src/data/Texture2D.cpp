#include "Texture2D.h"
#include "core/Engine.h"
#include "render/Renderer.h"

namespace CGE
{
	namespace vk = VULKAN_HPP_NAMESPACE;

	Texture2D::Texture2D(const HashString& inPath, bool inUsesAlpha /*= false*/, bool inFlipVertical /*= true*/, bool inLinear /*= true*/, bool inGenMips /*= true*/)
		: TextureData(inPath, inUsesAlpha, inFlipVertical, inLinear, inGenMips)
	{
	
	}
	
	Texture2D::~Texture2D()
	{
	
	}
	
	ImageCreateInfo Texture2D::GetImageInfo()
	{
		ImageCreateInfo createInfo;
	
		createInfo.setArrayLayers(1);
		createInfo.setFormat(Format::eR8G8B8A8Unorm);
		createInfo.setImageType(vk::ImageType::e2D);
		createInfo.setInitialLayout(ImageLayout::eUndefined);
		createInfo.setSamples(vk::SampleCountFlagBits::e1);
		createInfo.setMipLevels(genMips ? 12 : 1); // 12 just in case
		createInfo.setSharingMode(SharingMode::eExclusive);
		// ignored if exclusive mode is used, see Vulkan 1.2 spec
		//createInfo.setQueueFamilyIndexCount(1); 
		//createInfo.setPQueueFamilyIndices(queueFailyIndices);
		createInfo.setTiling(vk::ImageTiling::eOptimal);
		createInfo.setFlags(vk::ImageCreateFlags());
		createInfo.setExtent(vk::Extent3D(width, height, 1));
		createInfo.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc);
	
		return createInfo;
	}
	
	ImageView Texture2D::CreateImageView(ImageSubresourceRange range)
	{
		return image.CreateView(range, ImageViewType::e2D);
	}
	
}
