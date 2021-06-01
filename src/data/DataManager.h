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
#include <atomic>
#include <mutex>

namespace CGE
{

	struct ResourceRecord
	{
		std::atomic<int> useCounter;
		ResourcePtr resource;
	};
	
	class DataManager
	{
	public:
		static DataManager* GetInstance();
		static void ShutdownInstance();
		void CleanupResources();
	
		bool HasResource(HashString inKey);
		bool AddResource(ResourcePtr inValue);
		bool DeleteResource(ResourcePtr inValue);
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
		std::mutex m_mutex;
		std::map<HashString, ResourcePtr> m_resourcesTable;
		std::map<HashString, std::map<HashString, ResourcePtr>> m_resourcesMap;
	private:
		static DataManager* m_instance;
	
		DataManager();
		DataManager(const DataManager& inOther) {}
		void operator=(const DataManager& inOther) {}
		virtual ~DataManager();
	
		ResourcePtr GetResource(HashString inKey, std::map<HashString, ResourcePtr>& inMap);
	};
	
	//===========================================================================================
	// templated definitions
	//===========================================================================================
	
	template<class T>
	inline std::shared_ptr<T> DataManager::GetResource(HashString inKey)
	{
		return std::dynamic_pointer_cast<T>(GetResource(inKey));
	}
	
	//-----------------------------------------------------------------------------------
	
	template<class T>
	inline std::shared_ptr<T> DataManager::GetResourceByType(HashString inKey)
	{
		std::map<HashString, std::map<HashString, ResourcePtr>>::iterator it = m_resourcesMap.find(Class::Get<T>().GetName());
		if (it != m_resourcesMap.end())
		{
			return std::dynamic_pointer_cast<T>( GetResource(inKey, m_resourcesMap[inKey]) );
		}
		return nullptr;
	}
	
	//-----------------------------------------------------------------------------------
	
	template<class T, typename ...ArgTypes>
	inline std::shared_ptr<T> DataManager::RequestResourceByType(HashString inKey, ArgTypes&& ...args)
	{
		HashString className = Class::Get<T>().GetName();
		std::map<HashString, ResourcePtr>& resourceTypeMap = m_resourcesMap[className];
		std::map<HashString, ResourcePtr>::iterator it = resourceTypeMap.find(inKey);
		if (it != resourceTypeMap.end())
		{
			return std::dynamic_pointer_cast<T>(GetResource(inKey, resourceTypeMap));
		}
		std::shared_ptr<T> resource = ObjectBase::NewObject<T>(inKey, std::forward<ArgTypes>(args)...);
		if (resource.get())
		{
			resource->Create();
		}
		return resource;
	}
	
	//-----------------------------------------------------------------------------------
	
	template<class T, typename ...ArgTypes>
	static std::shared_ptr<T> DataManager::RequestResourceType(HashString inKey, ArgTypes&& ...args)
	{
		return m_instance->RequestResourceByType<T>(inKey, std::forward<ArgTypes>(args)...);
	}
}

