
#include "data/Resource.h"
#include "data/DataManager.h"
#include <assert.h>

namespace CGE
{
	Resource::Resource(HashString inId)
		: ObjectBase()
		, m_id{ inId }
	{
		assert(!DataManager::GetInstance()->HasResource(m_id));
	}
	
	void Resource::SetValid(bool inValid)
	{
		m_isValidFlag = inValid;
	}
	
	Resource::Resource()
		: ObjectBase{}
		, m_id{HashString::NONE}
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
		m_isValidFlag = false;
		// TODO: ask data manager to destroy this resource ???
		ObjectBase::OnDestroy();
	}
	
	HashString Resource::GetResourceId()
	{
		return m_id;
	}
	
	//bool Resource::Destroy()
	//{
	//	DataManager::GetInstance()->DecreaseUsageCount(m_id);
	//	return true;
	//}

	bool Resource::IsValid()
	{
		return m_isValidFlag;
	}
	
}
