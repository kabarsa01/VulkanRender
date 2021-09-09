#include "RenderPassResourceTable.h"


namespace CGE
{

	RenderPassResourceTable::RenderPassResourceTable()
	{
		m_resources.reserve(32);
	}

	void RenderPassResourceTable::AddResource(HashString key, const std::vector<ResourcePtr> resourceVector)
	{
		m_resources[key] = resourceVector;
	}

	std::vector<ResourcePtr> RenderPassResourceTable::GetResource(HashString key)
	{
		if (m_resources.find(key) != m_resources.end())
		{
			return m_resources[key];
		}
		return {};
	}

}

