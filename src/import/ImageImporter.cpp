#include "ImageImporter.h"
#include "stb/stb_image.h"

void ImageImporter::ImportImage(const std::string& inPath, bool inUsesAlpha /*= false*/, bool inFlipVertical /*= true*/, bool inLinear /*= true*/)
{
	unsigned char* data;
	int width;
	int height;
	int numChannels;
	stbi_set_flip_vertically_on_load(inFlipVertical);
	data = stbi_load(inPath.c_str(), &width, &height, &numChannels, 0);

//	return Data != nullptr;
}

