#pragma once

#include <map>
#include <set>
#include <string>
#include <memory>

#include "core/ObjectBase.h"
#include "core/Class.h"
#include "common/HashString.h"
#include "data/Resource.h"

using namespace std;

class DataManager
{
public:
	static DataManager* GetInstance();
	static void ShutdownInstance();

	bool AddResource(HashString InKey, shared_ptr<Resource> InValue);
	bool AddResource(ResourcePtr InValue);
	bool DeleteResource(HashString InKey, shared_ptr<Resource> InValue);
	bool DeleteResource(ResourcePtr InValue);
	bool IsResourcePresent(HashString InKey);
	shared_ptr<Resource> GetResource(HashString InKey);
	template<class T>
	shared_ptr<T> GetResource(HashString InKey);
	template<class T>
	shared_ptr<T> GetResourceByType(HashString InKey);
	template<class T, typename ...ArgTypes>
	shared_ptr<T> RequestResourceByType(HashString InKey, ArgTypes ...Args);
protected:
	map<HashString, ResourcePtr> ResourcesTable;
	map<HashString, map<HashString, ResourcePtr>> ResourcesMap;
private:
	static DataManager* Instance;

	DataManager();
	virtual ~DataManager();

	ResourcePtr GetResource(HashString InKey, map<HashString, ResourcePtr>& InMap);
};

//===========================================================================================
// templated definitions
//===========================================================================================

template<class T>
inline shared_ptr<T> DataManager::GetResource(HashString InKey)
{
	return dynamic_pointer_cast<T>(GetResource(InKey));
}

//-----------------------------------------------------------------------------------

template<class T>
inline shared_ptr<T> DataManager::GetResourceByType(HashString InKey)
{
	map<HashString, map<HashString, ResourcePtr>>::iterator It = ResourcesMap.find(Class::Get<T>().GetName());
	if (It != ResourcesMap.end())
	{
		return dynamic_pointer_cast<T>( GetResource(InKey, ResourcesMap[InKey]) );
	}
	return nullptr;
}

//-----------------------------------------------------------------------------------

template<class T, typename ...ArgTypes>
inline shared_ptr<T> DataManager::RequestResourceByType(HashString InKey, ArgTypes ...Args)
{
	HashString ClassName = Class::Get<T>().GetName();
	map<HashString, ResourcePtr>& ResourceTypeMap = ResourcesMap[ClassName];
	map<HashString, ResourcePtr>::iterator It = ResourceTypeMap.find(InKey);
	if (It != ResourceTypeMap.end())
	{
		return dynamic_pointer_cast<T>(GetResource(InKey, ResourceTypeMap));
	}
	shared_ptr<T> Resource = ObjectBase::NewObject<T, ArgTypes...>(Args...);
	if (Resource.get())
	{
		Resource->Load();
	}
	return Resource;
}

