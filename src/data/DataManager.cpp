
#include "data/DataManager.h"
#include "core/Class.h"

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
	map<HashString, ResourcePtr>::iterator it = resourcesTable.begin();
	for (; it != resourcesTable.end(); it++)
	{
		it->second->Cleanup();
	}
	resourcesTable.clear();
	resourcesMap.clear();
}

bool DataManager::AddResource(HashString inKey, shared_ptr<Resource> inValue)
{
	if (inValue && ( resourcesTable.find(inKey) == resourcesTable.end() ))
	{
		resourcesTable[inKey] = inValue;
		resourcesMap[inValue->GetClass().GetName()][inKey] = inValue;
		return true;
	}

	return false;
}

bool DataManager::AddResource(ResourcePtr inValue)
{
	return AddResource(inValue->GetResourceId(), inValue);
}

bool DataManager::DeleteResource(HashString inKey, shared_ptr<Resource> inValue)
{
	if (inValue && (resourcesTable.find(inKey) != resourcesTable.end()))
	{
		resourcesTable.erase(inKey);
		resourcesMap[inValue->GetClass().GetName()].erase(inKey);
		return true;
	}

	return false;
}

bool DataManager::DeleteResource(ResourcePtr inValue)
{
	return DeleteResource(inValue->GetResourceId(), inValue);
}

bool DataManager::IsResourcePresent(HashString inKey)
{
	return resourcesTable.find(inKey) != resourcesTable.end();
}

std::shared_ptr<Resource> DataManager::GetResource(HashString inKey)
{
	return GetResource(inKey, resourcesTable);
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


