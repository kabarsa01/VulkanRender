#pragma once

#include "data/Resource.h"
#include "render/resources/VulkanImage.h"
#include <string>

class TextureData : public Resource
{
public:
	TextureData(const HashString& inPath, bool inUsesAlpha = false, bool inFlipVertical = true, bool inLinear = true);
	virtual ~TextureData();

	virtual bool Load() override;
	virtual bool Unload() override;
protected:
	VulkanImage image;

	std::string path;
	bool usedAlpha;
	bool flipVertical;
	bool linear;

	int width;
	int height;
	int numChannels;

	virtual ImageCreateInfo GetImageInfo() = 0;
private:
	TextureData();
};
