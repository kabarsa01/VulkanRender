#include "VulkanImage.h"
#include "core/Engine.h"


VulkanImage::VulkanImage()
{

}

VulkanImage::~VulkanImage()
{
	Destroy();
}

void VulkanImage::Create()
{
	if (image)
	{
		return;
	}
	device = Engine::GetRendererInstance()->GetVulkanDevice();
	image = device.createImage(createInfo);
}

void VulkanImage::Destroy()
{
	if (image)
	{
		device.destroyImage(image);
		image = nullptr;
	}
}

Image& VulkanImage::GetImage()
{
	return image;
}

const Image& VulkanImage::GetImage() const
{
	return image;
}

MemoryRequirements VulkanImage::GetMemoryRequirements()
{
	return device.getImageMemoryRequirements(image);
}

