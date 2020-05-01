#include "render/shader/Shader.h"
#include <fstream>
#include <streambuf>
#include "spirv_cross/spirv_glsl.hpp"
#include "core/Engine.h"

Shader::Shader(const HashString& inPath)
	: Resource(inPath)
{
	filePath = inPath.GetString();
	shaderModule = nullptr;
}

Shader::~Shader()
{
	DestroyShaderModule();
}

bool Shader::Load()
{
	if (shaderModule)
	{
		return true;
	}

	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	binary.clear();

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	binary.resize(size);
	file.seekg(0, std::ios::beg);

	file.read(binary.data(), size);
	file.close();

	GetBindings();
	CreateShaderModule();
	return true;
}

bool Shader::Cleanup()
{
	DestroyShaderModule();
	return true;
}

ShaderModule Shader::GetShaderModule()
{
	return shaderModule;
}

void Shader::DestroyShaderModule()
{
	if (shaderModule)
	{
		Engine::GetRendererInstance()->GetVulkanDevice().GetDevice().destroyShaderModule(shaderModule);
		shaderModule = nullptr;
	}
}

const std::vector<char>& Shader::GetCode() const
{
	return binary;
}

std::vector<BindingInfo>& Shader::GetBindings()
{
	if (bindings.size() == 0)
	{
		SPIRV_CROSS_NAMESPACE::CompilerGLSL spirv(reinterpret_cast<const uint32_t*>(binary.data()), binary.size() / sizeof(uint32_t));
		SPIRV_CROSS_NAMESPACE::ShaderResources resources = spirv.get_shader_resources();

		// Get all sampled images in the shader.
		for (SPIRV_CROSS_NAMESPACE::Resource& resource : resources.uniform_buffers)
		{
			BindingInfo info;
			info.descriptorType = DescriptorType::eUniformBuffer;

			printf("uniform buffer name is %s \n", resource.name.c_str());

			info.set = spirv.get_decoration(resource.id, spv::DecorationDescriptorSet);
			info.binding = spirv.get_decoration(resource.id, spv::DecorationBinding);
			info.name = spirv.get_name(resource.id);
			info.typeName = resource.name;
			info.count = spirv.get_decoration(resource.id, spv::DecorationArrayStride);

			SPIRV_CROSS_NAMESPACE::SPIRType type = spirv.get_type(resource.type_id);
			SPIRV_CROSS_NAMESPACE::SPIRType baseType = spirv.get_type(resource.base_type_id);

			uint32_t arraySize = 0;
			if (type.array.size() > 0)
			{
				arraySize = type.array[0];
			}

			printf("UBO %s at set = %u, binding = %u\n", info.name.c_str(), info.set, info.binding);
		}
	}

	return bindings;
}

void Shader::CreateShaderModule()
{
	if (shaderModule)
	{
		return;
	}

	ShaderModuleCreateInfo createInfo;
	// size in bytes, even though code is uint32_t*
	createInfo.setCodeSize(binary.size());
	createInfo.setPCode(reinterpret_cast<const uint32_t*>(binary.data()));

	shaderModule = Engine::GetRendererInstance()->GetVulkanDevice().GetDevice().createShaderModule(createInfo);
}

