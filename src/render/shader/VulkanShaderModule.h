#pragma once

#include "core\ObjectBase.h"
#include <memory>
#include <vector>
#include "vulkan\vulkan.hpp"

class Shader;

class VulkanShaderModule : public ObjectBase
{
public:
	VulkanShaderModule(const Shader& inShader);
	virtual ~VulkanShaderModule();

	inline VULKAN_HPP_NAMESPACE::ShaderModule GetShaderModule() { return shaderModule; }
protected:
	VULKAN_HPP_NAMESPACE::ShaderModule shaderModule;
};

typedef std::shared_ptr<VulkanShaderModule> ScopedShaderModulePtr;
