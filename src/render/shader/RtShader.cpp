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

	vk::ShaderStageFlagBits RtShader::GetStageFlags()
	{
		switch (m_type)
		{
		case ERtShaderType::RST_RAY_GEN:
			return vk::ShaderStageFlagBits::eRaygenKHR;
		case ERtShaderType::RST_MISS:
			return vk::ShaderStageFlagBits::eMissKHR;
		case ERtShaderType::RST_INTERSECT:
			return vk::ShaderStageFlagBits::eIntersectionKHR;
		case ERtShaderType::RST_ANY_HIT:
			return vk::ShaderStageFlagBits::eAnyHitKHR;
		case ERtShaderType::RST_CLOSEST_HIT:
			return vk::ShaderStageFlagBits::eClosestHitKHR;
		}

		return vk::ShaderStageFlagBits::eAll;
	}

}



