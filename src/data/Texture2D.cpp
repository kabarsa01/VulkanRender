#include "Texture2D.h"
#include "core/Engine.h"
#include "render/Renderer.h"

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
	createInfo.setImageType(ImageType::e2D);
	createInfo.setInitialLayout(ImageLayout::eUndefined);
	createInfo.setSamples(SampleCountFlagBits::e1);
	createInfo.setMipLevels(genMips ? 12 : 1); // 12 just in case
	createInfo.setSharingMode(SharingMode::eExclusive);
	// ignored if exclusive mode is used, see Vulkan 1.2 spec
	//createInfo.setQueueFamilyIndexCount(1); 
	//createInfo.setPQueueFamilyIndices(queueFailyIndices);
	createInfo.setTiling(ImageTiling::eOptimal);
	createInfo.setFlags(ImageCreateFlags());
	createInfo.setExtent(Extent3D(width, height, 1));
	createInfo.setUsage(ImageUsageFlagBits::eSampled | ImageUsageFlagBits::eTransferDst | ImageUsageFlagBits::eTransferSrc);

	return createInfo;
}

ImageView Texture2D::CreateImageView(ImageSubresourceRange range)
{
	return image.CreateView(range, ImageViewType::e2D);
}

