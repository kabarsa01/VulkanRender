
#include "data/Resource.h"
#include "data/DataManager.h"

Resource::Resource(HashString inId)
	: ObjectBase()
	, id{ inId }
{
}

void Resource::SetValid(bool inValid)
{
	isValidFlag = inValid;
}

Resource::Resource()
	: ObjectBase{}
	, id{HashString::NONE}
{

}

Resource::~Resource()
{
}

void Resource::OnInitialize()
{
	ObjectBase::OnInitialize();
	DataManager::GetInstance()->AddResource(id, get_shared_from_this<Resource>());
}

void Resource::OnDestroy()
{
	Cleanup();
	isValidFlag = false;
	// ask data manager to destroy this resource ???
	ObjectBase::OnDestroy();
}

HashString Resource::GetResourceId()
{
	return id;
}

bool Resource::IsValid()
{
	return isValidFlag;
}

