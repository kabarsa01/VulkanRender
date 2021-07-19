#pragma once

#include "data/Resource.h"
#include "render/resources/VulkanImage.h"
#include <string>

namespace CGE
{
	class TextureData : public Resource
	{
	public:
		TextureData(const HashString& inPath, bool inUsesAlpha = false, bool inFlipVertical = true, bool inLinear = true, bool inGenMips = true);
		virtual ~TextureData();
	
		virtual bool Create() override;
	
		void CreateFromExternal(const VulkanImage& inImage, const ImageView& inImageView, bool inCleanup = false);
		void CreateFromExternal(std::shared_ptr<TextureData> texture, bool inCleanup = false);
	
		inline VulkanImage& GetImage() { return image; }
		ImageView& GetImageView();
		ImageView GetImageView() const;
		vk::DescriptorImageInfo& GetDescriptorInfo(vk::ImageLayout layout);
		vk::DescriptorImageInfo GetDescriptorInfo(vk::ImageLayout layout) const;
	protected:
		VulkanImage image;
		ImageView imageView;
		vk::DescriptorImageInfo descriptorInfo;
	
		std::string path;
		bool useAlpha;
		bool flipVertical;
		bool linear;
		bool genMips;
		bool cleanup;
	
		int width;
		int height;
		int numChannels;
	
		virtual ImageCreateInfo GetImageInfo() = 0;
		virtual ImageView CreateImageView(ImageSubresourceRange range) = 0;

		virtual bool Destroy() override;
	private:
		TextureData();
	};

	typedef std::shared_ptr<TextureData> TextureDataPtr;
}
