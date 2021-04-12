
#include "data/DataManager.h"
#include "core/Class.h"

namespace CGE
{
	
	//---------------------------------------------------------------
	// using declarations and aliases
	//---------------------------------------------------------------
	
	template<typename T1, typename T2>
	using map = std::map<T1, T2>;
	
	template<typename T>
	using shared_ptr = std::shared_ptr<T>;
	
	//---------------------------------------------------------------
	
	DataManager* DataManager::instance = new DataManager();
	
	DataManager::DataManager()
	{
	
	}
	
	DataManager::~DataManager()
	{
	
	}
	
	DataManager* DataManager::GetInstance()
	{
		return instance;
	}
	
	void DataManager::ShutdownInstance()
	{
		instance->CleanupResources();
		if (instance != nullptr)
		{
			delete instance;
		}
	}
	
	void DataManager::CleanupResources()
	{
		map<HashString, ResourcePtr>::iterator it = resourceIdMap.begin();
		for (; it != resourceIdMap.end(); it++)
		{
			it->second->Cleanup();
		}
		resourceIdMap.clear();
		resourceTypesMap.clear();
	}
	
	bool DataManager::IsResourcePresent(HashString inKey)
	{
		return resourceIdMap.find(inKey) != resourceIdMap.end();
	}
	
	std::shared_ptr<Resource> DataManager::GetResource(HashString inKey)
	{
		return GetResource(inKey, resourceIdMap);
	}
	
	ResourcePtr DataManager::GetResource(HashString inKey, map<HashString, ResourcePtr>& inMap)
	{
		map<HashString, ResourcePtr>::iterator it = inMap.find(inKey);
		if (it != inMap.end())
		{
			return it->second;
		}
		return nullptr;
	}
	
	
}
