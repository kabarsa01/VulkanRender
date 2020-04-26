#pragma once

#include "data/Resource.h"
#include "Texture2D.h"

class MaterialData : public Resource
{
public:
	MaterialData(HashString inId);
	virtual ~MaterialData();

	void LoadResources();

	void SetShaderPath(const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath);
	void SetTexture(const std::string& inName, Texture2DPtr inTexture2D);
protected:
	std::string vertexShaderPath;
	std::string fragmentShaderPath;

	std::map<std::string, Texture2DPtr> textures2D;
};
