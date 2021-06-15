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

	class RtShader : public Shader
	{
	public:
		RtShader(const HashString& inPath, ERtShaderType type);
		virtual ~RtShader();

		ERtShaderType GetType() { return m_type; }
		uint32_t GetTypeIntegral() { return static_cast<uint32_t>(m_type); }
	private:
		ERtShaderType m_type;
	};

	typedef std::shared_ptr<RtShader> RtShaderPtr;

}

#endif
