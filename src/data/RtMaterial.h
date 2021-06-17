#ifndef __RT_MATERIAL_H__
#define __RT_MATERIAL_H__

#include <array>

#include "Resource.h"
#include "render/shader/RtShader.h"

namespace CGE
{

	class RtMaterial : public Resource
	{
	public:
		RtMaterial(const HashString& id);
		virtual ~RtMaterial();

		void LoadResources();

		void SetShader(ERtShaderType type, std::string path, std::string entrypoint);
		RtShaderPtr GetShader(ERtShaderType type);
		bool HasHitGroup();
	private:
		struct RtShaderRecord
		{
			std::string path;
			std::string entrypoint;
			RtShaderPtr shader;
		};

		std::array<RtShaderRecord, static_cast<uint8_t>(ERtShaderType::RST_MAX)> m_shaderRecords;
	};

	typedef std::shared_ptr<RtMaterial> RtMaterialPtr;

}

#endif
