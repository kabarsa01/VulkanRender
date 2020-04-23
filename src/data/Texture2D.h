#pragma once

#include "TextureData.h"

class Texture2D : public TextureData
{
public:
	Texture2D(const std::string& inPath, bool inUsesAlpha = false, bool inFlipVertical = true, bool inLinear = true);
	virtual ~Texture2D();
protected:
	ImageCreateInfo GetImageInfo() override;
private:
	Texture2D();
};
