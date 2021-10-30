#include "TextureData.h"
#include "stb/stb_image.h"
#include "core/Engine.h"
#include "render/Renderer.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::ImageAspectFlagBits;

	namespace
	{
		static const int DESIRED_CHANNELS_COUNT = 4;
	};
	
	TextureData::TextureData(const HashString& inPath, bool inUsesAlpha /*= false*/, bool inFlipVertical /*= true*/, bool inLinear /*= true*/, bool inGenMips /*= true*/)
		: Resource(inPath)
		, image()
		, imageView(nullptr)
		, genMips(inGenMips)
		, cleanup(true)
	{
		path = inPath.GetString();
		useAlpha = inUsesAlpha;
		flipVertical = inFlipVertical;
		linear = inLinear;
	}
	
	//TextureData::TextureData()
	//	: Resource(HashString::NONE)
	//{
	//
	//}
	
	TextureData::~TextureData()
	{
		Destroy();
	}
	
	bool TextureData::Create()
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
		imageView = CreateImageView(ImageSubresourceRange(ImageAspectFlagBits::eColor, 0, image.GetMips(), 0, 1));
	
		stbi_image_free(data);
	
		return true;
	}
	
	bool TextureData::Destroy()
	{
		if (!cleanup)
		{
			return false;
		}
	
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
	
	void TextureData::CreateFromExternal(const VulkanImage& inImage, ImageView inImageView, bool inCleanup/* = false*/)
	{
		image = inImage;
		imageView = inImageView;
		cleanup = inCleanup;
	}

	void TextureData::CreateFromExternal(std::shared_ptr<TextureData> texture, bool inCleanup/* = false*/)
	{
		image = texture->image;
		imageView = texture->imageView;
		cleanup = inCleanup;
	}
	
	ImageView& TextureData::GetImageView()
	{
		return imageView;
	}
		
	ImageView TextureData::GetImageView() const
	{
		return imageView;
	}

	vk::DescriptorImageInfo& TextureData::GetDescriptorInfo(vk::ImageLayout layout)
	{
		descriptorInfo.setImageLayout(layout);
		descriptorInfo.setImageView(GetImageView());

		return descriptorInfo;
	}

	vk::DescriptorImageInfo TextureData::GetDescriptorInfo(vk::ImageLayout layout) const
	{
		vk::DescriptorImageInfo descInfo;
		descInfo.setImageLayout(layout);
		descInfo.setImageView(GetImageView());

		return descriptorInfo;
	}

}
