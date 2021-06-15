#ifndef __RT_MATERIAL_H__
#define __RT_MATERIAL_H__

#include "Resource.h"

namespace CGE
{

	class RtMaterial : public Resource
	{
	public:
		RtMaterial(const HashString& id);
		virtual ~RtMaterial();
	};

}

#endif
