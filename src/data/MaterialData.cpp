#include "MaterialData.h"

MaterialData::MaterialData(HashString inId)
	: Resource(inId)
{

}

MaterialData::~MaterialData()
{

}

void MaterialData::LoadResources()
{

}

void MaterialData::SetShaderPath(const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath)
{
	vertexShaderPath = inVertexShaderPath;
	fragmentShaderPath = inFragmentShaderPath;
}

void MaterialData::SetTexture(const std::string& inName, Texture2DPtr inTexture2D)
{
	textures2D[inName] = inTexture2D;
}

