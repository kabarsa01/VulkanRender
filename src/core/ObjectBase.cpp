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
	
	const Class & ObjectBase::GetClass()
	{
		return * instanceClass;
	}
	
	void ObjectBase::OnInitialize()
	{
		instanceClass = Class::GetClass(this);
	}
	
	void ObjectBase::OnDestroy()
	{
	}
	
}
