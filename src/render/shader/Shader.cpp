#include "Shader.h"
#include <fstream>
#include <streambuf>
#include "shaderc.h"

Shader::Shader()
	: ObjectBase()
{

}

Shader::~Shader()
{

}

void Shader::LoadText(const std::string& inFilePath)
{
	filePath = inFilePath;

	std::ifstream inputStream(filePath);
	code.clear();

	inputStream.seekg(0, std::ios::end);
	code.reserve(inputStream.tellg());
	inputStream.seekg(0, std::ios::beg);

	code.assign((std::istreambuf_iterator<char>(inputStream)), std::istreambuf_iterator<char>());
}

bool Shader::Compile()
{
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	shaderc_compilation_result* result = shaderc_compile_into_spv(
	    compiler, code.c_str(), code.length(),
	    shaderc_glsl_vertex_shader, filePath.c_str(), "main", nullptr);
	// Do stuff with compilation results.
//	result->

	shaderc_result_release(result);
	shaderc_compiler_release(compiler);

	return true;
}

