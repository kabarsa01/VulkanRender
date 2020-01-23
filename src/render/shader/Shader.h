#pragma once

#include "core\ObjectBase.h"
#include <memory>
#include <string>
#include <vector>

class Shader : public ObjectBase
{
public:
	Shader();
	virtual ~Shader();

	void LoadText(const std::string& inFilePath);
	bool Compile();
protected:
	std::string filePath;
	std::string code;
	std::vector<unsigned char> binary;
};

typedef std::shared_ptr<Shader> ShaderPtr;
