#pragma once

#include "TextureData.h"
#include <memory>

class Texture2D : public TextureData
{
public:
	Texture2D(const HashString& inPath, bool inUsesAlpha = false, bool inFlipVertical = true, bool inLinear = true, bool inGenMips = true);
	virtual ~Texture2D();
protected:
	ImageCreateInfo GetImageInfo() override;
	ImageView CreateImageView(ImageSubresourceRange range) override;
private:
	Texture2D();
};

typedef std::shared_ptr<Texture2D> Texture2DPtr;

