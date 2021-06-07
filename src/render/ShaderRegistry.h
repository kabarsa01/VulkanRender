#ifndef __SHADER_REGISTRY_H__
#define __SHADER_REGISTRY_H__

#include <vector>
#include <mutex>
#include <set>

#include "shader/Shader.h"
#include "utils/Singleton.h"

namespace CGE
{

	class ShaderRegistry
	{
	public:
		ShaderRegistry();
		~ShaderRegistry();


	private:
		friend class Shader;

		std::vector<ShaderPtr> m_shaderSet;

		void AddShader(ShaderPtr shader);
		void RemoveShader(ShaderPtr shader);
	};

}

#endif
