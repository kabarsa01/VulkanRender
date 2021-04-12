#pragma once

#include <map>
#include <set>
#include <string>
#include <memory>

#include "core/ObjectBase.h"
#include "core/Class.h"
#include "common/HashString.h"
#include "data/Resource.h"
#include <type_traits>

namespace CGE
{
	
	class DataManager
	{
	public:
		static DataManager* GetInstance();
		static void ShutdownInstance();
		void CleanupResources();
	
		template<typename T>
		bool AddResource(HashString inKey, std::shared_ptr<T> inValue);
		template<typename T>
		bool AddResource(std::shared_ptr<T> inValue);
		template<typename T>
		bool DeleteResource(HashString inKey, std::shared_ptr<T> inValue);
		template<typename T>
		bool DeleteResource(std::shared_ptr<T> inValue);
		bool IsResourcePresent(HashString inKey);
		std::shared_ptr<Resource> GetResource(HashString inKey);
		template<class T>
		std::shared_ptr<T> GetResource(HashString inKey);
		template<class T>
		std::shared_ptr<T> GetResourceByType(HashString inKey);
		template<class T, typename ...ArgTypes>
		std::shared_ptr<T> RequestResourceByType(HashString inKey, ArgTypes&& ...args);
		template<class T, typename ...ArgTypes>
		static std::shared_ptr<T> RequestResourceType(HashString inKey, ArgTypes&& ...args);
	protected:
		std::map<HashString, ResourcePtr> resourceIdMap;
		std::map<ClassBase::ClassId, std::map<HashString, ResourcePtr>> resourceTypesMap;
	private:
		static DataManager* instance;
	
		DataManager();
		DataManager(const DataManager& inOther) {}
		void operator=(const DataManager& inOther) {}
		virtual ~DataManager();
	
		ResourcePtr GetResource(HashString inKey, std::map<HashString, ResourcePtr>& inMap);
	};

	//===========================================================================================
	// templated definitions
	//===========================================================================================
	
	template<typename T>
	bool DataManager::AddResource(HashString inKey, std::shared_ptr<T> inValue)
	{
		if (inValue && (resourceIdMap.find(inKey) == resourceIdMap.end()))
		{
			resourceIdMap[inKey] = inValue;
			resourceTypesMap[Class<T>::Id()][inKey] = inValue;
			return true;
		}

		return false;
	}

	template<typename T>
	bool DataManager::AddResource(std::shared_ptr<T> inValue)
	{
		return AddResource(inValue->GetResourceId(), inValue);
	}

	template<typename T>
	bool DataManager::DeleteResource(HashString inKey, std::shared_ptr<T> inValue)
	{
		if (inValue && (resourceIdMap.find(inKey) != resourceIdMap.end()))
		{
			resourceIdMap.erase(inKey);
			resourceTypesMap[Class<T>::Id()].erase(inKey);
			return true;
		}

		return false;
	}

	template<typename T>
	bool DataManager::DeleteResource(std::shared_ptr<T> inValue)
	{
		return DeleteResource(inValue->GetResourceId(), inValue);
	}
	
	template<class T>
	inline std::shared_ptr<T> DataManager::GetResource(HashString inKey)
	{
		return std::dynamic_pointer_cast<T>(GetResource(inKey));
	}
	
	//-----------------------------------------------------------------------------------
	
	template<class T>
	inline std::shared_ptr<T> DataManager::GetResourceByType(HashString inKey)
	{
		std::map<HashString, std::map<HashString, ResourcePtr>>::iterator it = resourceTypesMap.find(Class<T>::Id());
		if (it != resourceTypesMap.end())
		{
			return std::dynamic_pointer_cast<T>( GetResource(inKey, resourceTypesMap[inKey]) );
		}
		return nullptr;
	}
	
	//-----------------------------------------------------------------------------------
	
	template<class T, typename ...ArgTypes>
	inline std::shared_ptr<T> DataManager::RequestResourceByType(HashString inKey, ArgTypes&& ...args)
	{
		std::map<HashString, ResourcePtr>& resourceType = resourceTypesMap[Class<T>::Id()];
		std::map<HashString, ResourcePtr>::iterator it = resourceType.find(inKey);
		if (it != resourceType.end())
		{
			return std::dynamic_pointer_cast<T>(GetResource(inKey, resourceType));
		}
		std::shared_ptr<T> resource = ObjectBase::NewObject<T>(inKey, std::forward<ArgTypes>(args)...);
		if (resource.get())
		{
			resource->Load();
		}
		return resource;
	}
	
	//-----------------------------------------------------------------------------------
	
	template<class T, typename ...ArgTypes>
	static std::shared_ptr<T> DataManager::RequestResourceType(HashString inKey, ArgTypes&& ...args)
	{
		return instance->RequestResourceByType<T>(inKey, std::forward<ArgTypes>(args)...);
	}
}

