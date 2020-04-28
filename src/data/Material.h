#pragma once

#include "data/Resource.h"
#include "Texture2D.h"

class Material : public Resource
{
public:
	Material(HashString inId);
	Material(HashString inId, const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath);
	virtual ~Material();

	void LoadResources();
	HashString GetShaderHash();

	void SetShaderPath(const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath);
	void SetTexture(const std::string& inName, Texture2DPtr inTexture2D);
protected:
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
	HashString shaderHash;

	std::map<std::string, Texture2DPtr> textures2D;
};

typedef std::shared_ptr<Material> MaterialPtr;

