#ifndef __RT_SHADER_H__
#define __RT_SHADER_H__

#include "Shader.h"

namespace CGE
{

	enum class ERtShaderType : uint8_t
	{
		RST_RAY_GEN = 0,
		RST_MISS,
		RST_INTERSECT,
		RST_ANY_HIT,
		RST_CLOSEST_HIT,
		RST_MAX
	};

	inline ERtShaderType FromInt(uint8_t value)
	{
		return static_cast<ERtShaderType>(value);
	}
	inline uint8_t ToInt(ERtShaderType type)
	{
		return static_cast<uint8_t>(type);
	}

	class RtShader : public Shader
	{
	public:
		RtShader(const HashString& inPath, ERtShaderType type);
		virtual ~RtShader();

		ERtShaderType GetType() { return m_type; }
		uint8_t GetTypeIntegral() { return static_cast<uint8_t>(m_type); }
		bool IsGeneral() { return (m_type == ERtShaderType::RST_RAY_GEN) || (m_type == ERtShaderType::RST_MISS); }
		vk::ShaderStageFlagBits GetStageFlags();
	private:
		ERtShaderType m_type;
	};

	typedef std::shared_ptr<RtShader> RtShaderPtr;

}

#endif
