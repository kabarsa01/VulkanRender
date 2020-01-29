#pragma once

#include "core\ObjectBase.h"
#include <memory>
#include <vector>
#include "vulkan\vulkan.hpp"

class Shader;

class ShaderModuleWrapper : public ObjectBase
{
public:
	ShaderModuleWrapper(const Shader& inShader);
	virtual ~ShaderModuleWrapper();

	inline VULKAN_HPP_NAMESPACE::ShaderModule GetShaderModule() { return shaderModule; }
protected:
	VULKAN_HPP_NAMESPACE::ShaderModule shaderModule;
};

typedef std::shared_ptr<ShaderModuleWrapper> ScopedShaderModulePtr;
