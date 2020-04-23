#include "TextureData.h"
#include "stb/stb_image.h"
#include "core/Engine.h"

TextureData::TextureData(const std::string& inPath, bool inUsesAlpha /*= false*/, bool inFlipVertical /*= true*/, bool inLinear /*= true*/)
	: Resource(inPath)
{
	path = inPath;
	usedAlpha = inUsesAlpha;
	flipVertical = inFlipVertical;
	linear = inLinear;
}

TextureData::TextureData()
	: Resource(HashString::NONE())
{

}

TextureData::~TextureData()
{
	if (image)
	{
		image.Destroy();
	}
	Unload();
}

bool TextureData::Load()
{
	unsigned char* data;
	stbi_set_flip_vertically_on_load(flipVertical);
	data = stbi_load(path.c_str(), &width, &height, &numChannels, 0);

	if (data == nullptr)
	{
		return false;
	}

	VulkanDevice& device = Engine::GetRendererInstance()->GetVulkanDevice();
	image.createInfo = GetImageInfo();
	image.Create(&device);
	image.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
	image.CreateStagingBuffer(reinterpret_cast<char*>(data));

	stbi_image_free(data);

	return true;
}

bool TextureData::Unload()
{
	if (image)
	{
		image.DestroyStagingBuffer();
	}
	return false;
}



