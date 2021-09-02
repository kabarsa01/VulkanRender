#ifndef __RENDER_PASS_RESOURCE_TABLE_H__
#define __RENDER_PASS_RESOURCE_TABLE_H__

#include <unordered_map>

#include "common/HashString.h"
#include "data/Resource.h"

namespace CGE
{

	class RenderPassResourceTable
	{
	public:
		RenderPassResourceTable() {}
		~RenderPassResourceTable() {}
	private:
		std::unordered_map<HashString, std::vector<ResourcePtr>> m_resources;
	};

}

#endif
 