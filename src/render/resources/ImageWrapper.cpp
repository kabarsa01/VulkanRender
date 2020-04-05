#include "ImageWrapper.h"
#include "core/Engine.h"


ImageWrapper::ImageWrapper()
{

}

ImageWrapper::~ImageWrapper()
{
	Destroy();
}

void ImageWrapper::Create()
{
	if (image)
	{
		return;
	}
	device = Engine::GetRendererInstance()->GetDevice();
	image = device.createImage(createInfo);
}

void ImageWrapper::Destroy()
{
	if (image)
	{
		device.destroyImage(image);
		image = nullptr;
	}
}

Image& ImageWrapper::GetImage()
{
	return image;
}

Image ImageWrapper::GetImage() const
{
	return image;
}

MemoryRequirements ImageWrapper::GetMemoryRequirements()
{
	return device.getImageMemoryRequirements(image);
}

