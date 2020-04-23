#include "Texture2D.h"

Texture2D::Texture2D(const std::string& inPath, bool inUsesAlpha /*= false*/, bool inFlipVertical /*= true*/, bool inLinear /*= true*/)
	: TextureData(inPath, inUsesAlpha, inFlipVertical, inLinear)
{

}

Texture2D::~Texture2D()
{

}

ImageCreateInfo Texture2D::GetImageInfo()
{
	ImageCreateInfo createInfo;

	image.createInfo.setArrayLayers(1);
	image.createInfo.setFormat(Format::eR8G8B8A8Srgb);
	image.createInfo.setImageType(ImageType::e2D);
	image.createInfo.setInitialLayout(ImageLayout::eUndefined);
	image.createInfo.setSamples(SampleCountFlagBits::e1);
	image.createInfo.setMipLevels(1);
	image.createInfo.setSharingMode(SharingMode::eExclusive);
	// ignored if exclusive mode is used, see Vulkan 1.2 spec
	//image.createInfo.setQueueFamilyIndexCount(1); 
	//image.createInfo.setPQueueFamilyIndices(queueFailyIndices);
	image.createInfo.setTiling(ImageTiling::eOptimal);
	image.createInfo.setFlags(ImageCreateFlags());
	image.createInfo.setExtent(Extent3D(width, height, 1));
	image.createInfo.setUsage(ImageUsageFlagBits::eSampled);

	return createInfo;
}

