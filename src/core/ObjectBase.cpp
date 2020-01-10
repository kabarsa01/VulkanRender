#include "core/ObjectBase.h"
#include "core/Class.h"

ObjectBase::ObjectBase()
{
}

ObjectBase::~ObjectBase()
{
	OnDestroy();
}

const Class & ObjectBase::GetClass()
{
	return * InstanceClass;
}

void ObjectBase::OnInitialize()
{
	InstanceClass = Class::GetClass(this);
}

void ObjectBase::OnDestroy()
{
}

