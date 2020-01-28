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
}

const std::vector<char>& Shader::GetCode() const
{
	return binary;
}


