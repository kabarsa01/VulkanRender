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

	/*const uint32_t *ir_, size_t word_count*/
	SPIRV_CROSS_NAMESPACE::CompilerGLSL spirv(reinterpret_cast<const uint32_t*>( binary.data() ), size / sizeof(uint32_t));
	SPIRV_CROSS_NAMESPACE::ShaderResources resources = spirv.get_shader_resources();

	// Get all sampled images in the shader.
	for (SPIRV_CROSS_NAMESPACE::Resource& resource : resources.uniform_buffers)
	{
		printf("uniform buffer name is %s \n", resource.name.c_str());
		unsigned set = spirv.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = spirv.get_decoration(resource.id, spv::DecorationBinding);

		std::string name = spirv.get_name(resource.id);
		printf("UBO %s at set = %u, binding = %u\n", name.c_str(), set, binding);

		// Some arbitrary remapping if we want.
		//spirv.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
	}

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

