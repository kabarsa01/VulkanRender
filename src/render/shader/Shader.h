#pragma once

#include "core\ObjectBase.h"
#include <memory>
#include <string>
#include <vector>
#include "vulkan\vulkan.hpp"
#include "data\Resource.h"

using namespace VULKAN_HPP_NAMESPACE;

class Shader : public Resource
{
public:
	Shader(const HashString& inPath);
	virtual ~Shader();

	virtual bool Load() override;
	virtual bool Unload() override;

	ShaderModule GetShaderModule();
	void DestroyShaderModule();
	const std::vector<char>& GetCode() const;
protected:
	std::string filePath;
	std::vector<char> binary;

	ShaderModule shaderModule;

	void CreateShaderModule();
};

typedef std::shared_ptr<Shader> ShaderPtr;
