#pragma once
#include <string>

namespace CGE
{
	class ImageImporter
	{
	public:
		void ImportImage(const std::string& inPath, bool inUsesAlpha = false, bool inFlipVertical = true, bool inLinear = true);
	};
}
