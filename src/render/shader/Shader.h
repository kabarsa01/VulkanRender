#pragma once

#include "core\ObjectBase.h"
#include <memory>
#include <string>
#include <vector>
#include "vulkan\vulkan.hpp"
#include "data\Resource.h"
#include "spirv_cross\spirv_cross.hpp"

using namespace VULKAN_HPP_NAMESPACE;

struct BindingInfo
{
	uint32_t set;
	uint32_t binding;
	uint32_t vectorSize;
	uint32_t numColumns;
	DescriptorType descriptorType;
	HashString name;
	HashString blockName;
	std::vector<uint32_t> arrayDimensions;

	bool IsArray()
	{
		return arrayDimensions.size() > 0;
	}
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

	std::vector<BindingInfo>& GetBindings(DescriptorType inDescriptorType);
protected:
	std::string filePath;
	std::vector<char> binary;
	std::map<DescriptorType, std::vector<BindingInfo>> bindings;

	ShaderModule shaderModule;

	void CreateShaderModule();
	std::vector<BindingInfo> ExtractBindingInfo(
		SPIRV_CROSS_NAMESPACE::SmallVector<SPIRV_CROSS_NAMESPACE::Resource>& inResources, 
		SPIRV_CROSS_NAMESPACE::Compiler& inCompiler,
		DescriptorType inDescriptorType);
	void ExtractBindingsInfo();
};

typedef std::shared_ptr<Shader> ShaderPtr;
