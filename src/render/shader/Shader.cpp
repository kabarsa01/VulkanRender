#include "render/shader/Shader.h"
#include <fstream>
#include <streambuf>
#include "spirv_cross/spirv_glsl.hpp"

Shader::Shader()
	: ObjectBase()
{

}

Shader::~Shader()
{

}

void Shader::Load(const std::string& inFilePath)
{
	filePath = inFilePath;

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
	spirv_cross::Compiler glsl(reinterpret_cast<const uint32_t*>( binary.data() ), size / sizeof(uint32_t));
	spirv_cross::ShaderResources resources = glsl.get_shader_resources();

	// Get all sampled images in the shader.
	for (SPIRV_CROSS_NAMESPACE::Resource& resource : resources.uniform_buffers)
	{
		printf("uniform buffer name is %s \n", resource.name.c_str());
		unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

		std::string name = glsl.get_name(resource.id);
		//uint32_t index = glsl.get_decoration(resource.id, spv::DecorationIndex);
		//std::string memberName = glsl.get_member_name(resource.type_id, index);
		printf("UBO %s at set = %u, binding = %u\n", name.c_str(), set, binding);
		glsl.get_decoration(resource.id, spv::DecorationUserSemantic);

		//// Modify the decoration to prepare it for GLSL.
		//glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);

		//// Some arbitrary remapping if we want.
		//glsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
	}
}

const std::vector<char>& Shader::GetCode() const
{
	return binary;
}


