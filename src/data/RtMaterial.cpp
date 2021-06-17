#include "RtMaterial.h"
#include "DataManager.h"

namespace CGE
{

	RtMaterial::RtMaterial(const HashString& id)
		: Resource(id)
	{
	}

	RtMaterial::~RtMaterial()
	{

	}

	void RtMaterial::LoadResources()
	{
		for (uint8_t idx = 0; idx < static_cast<uint8_t>(ERtShaderType::RST_MAX); idx++)
		{
			RtShaderRecord& rec = m_shaderRecords[idx];
			if (rec.shader)
			{
				continue;
			}
			RtShaderPtr shader = DataManager::RequestResourceType<RtShader>(rec.path, FromInt(idx));
			rec.shader = shader;
		}
	}

	void RtMaterial::SetShader(ERtShaderType type, std::string path, std::string entrypoint)
	{
		RtShaderRecord& rec = m_shaderRecords[static_cast<uint8_t>(type)];
		rec.path = path;
		rec.entrypoint = entrypoint;
	}

	RtShaderPtr RtMaterial::GetShader(ERtShaderType type)
	{
		return m_shaderRecords[ToInt(type)].shader;
	}

	bool RtMaterial::HasHitGroup()
	{
		return m_shaderRecords[ToInt(ERtShaderType::RST_INTERSECT)].shader
			|| m_shaderRecords[ToInt(ERtShaderType::RST_ANY_HIT)].shader
			|| m_shaderRecords[ToInt(ERtShaderType::RST_CLOSEST_HIT)].shader;
	}

}



