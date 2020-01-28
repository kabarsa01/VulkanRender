#pragma once

#include "core\ObjectBase.h"
#include <memory>
#include <string>
#include <vector>
#include "vulkan\vulkan.hpp"

class Shader : public ObjectBase
{
public:
	Shader();
	virtual ~Shader();

	void Load(const std::string& inFilePath);
	const std::vector<char>& GetCode() const;
protected:
	std::string filePath;
	std::vector<char> binary;
};

typedef std::shared_ptr<Shader> ShaderPtr;
