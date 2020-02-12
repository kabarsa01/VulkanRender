
#include "data/Resource.h"
#include "data/DataManager.h"

Resource::Resource(HashString InId)
	: ObjectBase()
	, Id{ InId }
{
}

void Resource::SetValid(bool InValid)
{
	IsValidFlag = InValid;
}

Resource::Resource()
	: ObjectBase{}
	, Id{HashString::NONE()}
{

}

Resource::~Resource()
{
}

void Resource::OnInitialize()
{
	ObjectBase::OnInitialize();
	DataManager::GetInstance()->AddResource(Id, get_shared_from_this<Resource>());
}

void Resource::OnDestroy()
{
	Unload();
	IsValidFlag = false;
	// ask data manager to destroy this resource ???
	ObjectBase::OnDestroy();
}

HashString Resource::GetResourceId()
{
	return Id;
}

bool Resource::IsValid()
{
	return IsValidFlag;
}

