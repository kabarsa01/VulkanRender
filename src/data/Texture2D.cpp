#include "Texture2D.h"
#include "core/Engine.h"

Texture2D::Texture2D(const HashString& inPath, bool inUsesAlpha /*= false*/, bool inFlipVertical /*= true*/, bool inLinear /*= true*/)
	: TextureData(inPath, inUsesAlpha, inFlipVertical, inLinear)
{

}

Texture2D::~Texture2D()
{

}

ImageCreateInfo Texture2D::GetImageInfo()
{
	ImageCreateInfo createInfo;

	createInfo.setArrayLayers(1);
	createInfo.setFormat(Format::eR8G8B8A8Srgb);
	createInfo.setImageType(ImageType::e2D);
	createInfo.setInitialLayout(ImageLayout::eUndefined);
	createInfo.setSamples(SampleCountFlagBits::e1);
	createInfo.setMipLevels(1);
	createInfo.setSharingMode(SharingMode::eExclusive);
	// ignored if exclusive mode is used, see Vulkan 1.2 spec
	//createInfo.setQueueFamilyIndexCount(1); 
	//createInfo.setPQueueFamilyIndices(queueFailyIndices);
	createInfo.setTiling(ImageTiling::eOptimal);
	createInfo.setFlags(ImageCreateFlags());
	createInfo.setExtent(Extent3D(width, height, 1));
	createInfo.setUsage(ImageUsageFlagBits::eSampled);

	return createInfo;
}

ImageView Texture2D::CreateImageView()
{
	ImageViewCreateInfo imageViewInfo;
	imageViewInfo.setComponents(ComponentMapping());
	imageViewInfo.setFormat(image.createInfo.format);
	imageViewInfo.setImage(image);
	imageViewInfo.setSubresourceRange(ImageSubresourceRange(ImageAspectFlagBits::eColor, 0, 1, 0, 1));
	imageViewInfo.setViewType(ImageViewType::e2D);

	return Engine::GetRendererInstance()->GetDevice().createImageView(imageViewInfo);
}

