#include "TextureData.h"
#include "stb/stb_image.h"
#include "core/Engine.h"
#include "render/Renderer.h"

namespace
{
	static const int DESIRED_CHANNELS_COUNT = 4;
};

TextureData::TextureData(const HashString& inPath, bool inUsesAlpha /*= false*/, bool inFlipVertical /*= true*/, bool inLinear /*= true*/)
	: Resource(inPath)
	, imageView(nullptr)
{
	path = inPath.GetString();
	useAlpha = inUsesAlpha;
	flipVertical = inFlipVertical;
	linear = inLinear;
}

TextureData::TextureData()
	: Resource(HashString::NONE)
{

}

TextureData::~TextureData()
{
	Cleanup();
}

bool TextureData::Load()
{
	unsigned char* data;
	stbi_set_flip_vertically_on_load(flipVertical);
	data = stbi_load(path.c_str(), &width, &height, &numChannels, DESIRED_CHANNELS_COUNT);

	if (data == nullptr)
	{
		return false;
	}

	VulkanDevice& device = Engine::GetRendererInstance()->GetVulkanDevice();
	image.createInfo = GetImageInfo();
	image.Create(&device);
	image.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
	image.CreateStagingBuffer(reinterpret_cast<char*>(data));
	imageView = CreateImageView();

	stbi_image_free(data);

	return true;
}

bool TextureData::Cleanup()
{
	if (imageView)
	{
		Engine::GetRendererInstance()->GetDevice().destroyImageView(imageView);
		imageView = nullptr;
	}
	if (image)
	{
		image.Destroy();
		return true;
	}
	return false;
}

ImageView& TextureData::GetImageView()
{
	return imageView;
}





