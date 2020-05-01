#pragma once

#include "core\ObjectBase.h"
#include <memory>
#include <string>
#include <vector>
#include "vulkan\vulkan.hpp"
#include "data\Resource.h"

using namespace VULKAN_HPP_NAMESPACE;

struct BindingInfo
{
	uint32_t set;
	uint32_t binding;
	uint32_t count;
	std::string name;
	std::string typeName;
	DescriptorType descriptorType;
};

class Shader : public Resource
{
public:
	Shader(const HashString& inPath);
	virtual ~Shader();

	virtual bool Load() override;
	virtual bool Cleanup() override;

	ShaderModule GetShaderModule();
	void DestroyShaderModule();
	const std::vector<char>& GetCode() const;

	std::vector<BindingInfo>& GetBindings();
protected:
	std::string filePath;
	std::vector<char> binary;
	std::vector<BindingInfo> bindings;

	ShaderModule shaderModule;

	void CreateShaderModule();
};

typedef std::shared_ptr<Shader> ShaderPtr;
