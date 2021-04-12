#include "core/ObjectBase.h"
#include "core/Class.h"

namespace CGE
{
	ObjectBase::ObjectBase()
	{
	}
	
	ObjectBase::~ObjectBase()
	{
		OnDestroy();
	}
	
	void ObjectBase::OnInitialize()
	{
	}
	
	void ObjectBase::OnDestroy()
	{
	}
	
}
