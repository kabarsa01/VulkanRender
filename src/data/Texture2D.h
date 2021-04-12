#pragma once

#include "TextureData.h"
#include <memory>
#include "core/Class.h"

namespace CGE
{
	class Texture2D : public TextureData, public ClassType<Texture2D>
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
	
}
