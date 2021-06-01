
#include "data/Resource.h"
#include "data/DataManager.h"
#include <assert.h>

namespace CGE
{
	Resource::Resource(HashString inId)
		: ObjectBase()
		, id{ inId }
	{
		assert(!DataManager::GetInstance()->HasResource(id));
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
		DataManager::GetInstance()->AddResource(get_shared_from_this<Resource>());
	}
	
	void Resource::OnDestroy()
	{
		Destroy();
		isValidFlag = false;
		// TODO: ask data manager to destroy this resource ???
		ObjectBase::OnDestroy();
	}
	
	HashString Resource::GetResourceId()
	{
		return id;
	}
	
	bool Resource::Destroy()
	{
		return DataManager::GetInstance()->DeleteResource(get_shared_from_this<Resource>());
	}

	bool Resource::IsValid()
	{
		return isValidFlag;
	}
	
}
