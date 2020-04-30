#pragma once

#include "data/Resource.h"
#include "Texture2D.h"
#include "render/shader/Shader.h"

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
	inline ShaderPtr GetVertexShader() { return vertexShader; }
	inline ShaderPtr GetFragmentShader() { return fragmentShader; }

	bool Load() override;
	bool Cleanup() override;
protected:
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
	HashString shaderHash;

	ShaderPtr vertexShader;
	ShaderPtr fragmentShader;
	std::map<std::string, Texture2DPtr> textures2D;
};

typedef std::shared_ptr<Material> MaterialPtr;

