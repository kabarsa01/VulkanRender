#include "Shader.h"
#include <fstream>
#include <streambuf>

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
	return true;
}

