#pragma once

#include "data/Resource.h"
#include "render/resources/VulkanImage.h"
#include <string>

class TextureData : public Resource
{
public:
	TextureData(const HashString& inPath, bool inUsesAlpha = false, bool inFlipVertical = true, bool inLinear = true, bool inGenMips = true);
	virtual ~TextureData();

	virtual bool Load() override;
	virtual bool Cleanup() override;

	void CreateFromExternal(const VulkanImage& inImage, const ImageView& inImageView, bool inCleanup = false);

	inline VulkanImage& GetImage() { return image; }
	ImageView& GetImageView();
protected:
	VulkanImage image;
	ImageView imageView;

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
private:
	TextureData();
};
