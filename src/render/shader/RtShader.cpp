#include "RtShader.h"

namespace CGE
{

	RtShader::RtShader(const HashString& inPath, ERtShaderType type)
		: Shader(inPath)
		, m_type(type)
	{
	}

	RtShader::~RtShader()
	{

	}

}



