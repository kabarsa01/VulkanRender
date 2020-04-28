#include "Material.h"

Material::Material(HashString inId)
	: Resource(inId)
	, shaderHash(HashString::NONE())
{

}

Material::Material(HashString inId, const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath)
	: Resource(inId)
	, vertexShaderPath(inVertexShaderPath)
	, fragmentShaderPath(inFragmentShaderPath)
	, shaderHash(inVertexShaderPath + inFragmentShaderPath)
{

}

Material::~Material()
{

}

void Material::LoadResources()
{

}

HashString Material::GetShaderHash()
{
	return shaderHash;
}

void Material::SetShaderPath(const std::string& inVertexShaderPath, const std::string& inFragmentShaderPath)
{
	vertexShaderPath = inVertexShaderPath;
	fragmentShaderPath = inFragmentShaderPath;
	shaderHash = HashString(vertexShaderPath + fragmentShaderPath);
}

void Material::SetTexture(const std::string& inName, Texture2DPtr inTexture2D)
{
	textures2D[inName] = inTexture2D;
}

