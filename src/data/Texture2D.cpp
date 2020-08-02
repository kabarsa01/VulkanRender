#include "Texture2D.h"
#include "core/Engine.h"
#include "render/Renderer.h"

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
	createInfo.setFormat(Format::eR8G8B8A8Unorm);
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
	createInfo.setUsage(ImageUsageFlagBits::eSampled | ImageUsageFlagBits::eTransferDst);

	return createInfo;
}

ImageView Texture2D::CreateImageView()
{
	return image.CreateView({ ImageAspectFlagBits::eColor, 0, 1, 0, 1 }, ImageViewType::e2D);
}

