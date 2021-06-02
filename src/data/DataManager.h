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
#include <unordered_map>

namespace CGE
{
	
	class DataManager
	{
	public:
		static DataManager* GetInstance();
		static void ShutdownInstance();
		void CleanupResources();
	
		bool HasResource(HashString inKey);
		bool AddResource(ResourcePtr inValue);
		std::shared_ptr<Resource> GetResource(HashString inKey);
		template<class T>
		std::shared_ptr<T> GetResourceByType(HashString inKey);
		template<class T, typename ...ArgTypes>
		std::shared_ptr<T> RequestResourceByType(HashString inKey, ArgTypes&& ...args);
		template<class T, typename ...ArgTypes>
		static std::shared_ptr<T> RequestResourceType(HashString inKey, ArgTypes&& ...args);
	protected:
		std::mutex m_mutex;
		std::unordered_map<HashString, ResourcePtr> m_resourcesTable;
		std::unordered_map<HashString, std::unordered_map<HashString, ResourcePtr>> m_resourcesMap;
	private:
		static DataManager* m_instance;
	
		DataManager();
		DataManager(const DataManager& inOther) {}
		void operator=(const DataManager& inOther) {}
		virtual ~DataManager();
	
		ResourcePtr GetResource(HashString inKey, std::unordered_map<HashString, ResourcePtr>& inMap);
		bool DeleteResource(ResourcePtr inValue);
		bool DeleteResource(HashString key);
	};
	
	//===========================================================================================
	// templated definitions
	//===========================================================================================
	
	template<class T>
	inline std::shared_ptr<T> DataManager::GetResourceByType(HashString inKey)
	{
		std::scoped_lock<std::mutex> lock(m_mutex);
		auto it = m_resourcesMap.find(Class::Get<T>().GetName());
		if (it != m_resourcesMap.end())
		{
			return std::dynamic_pointer_cast<T>( GetResource(inKey, it->second) );
		}
		return nullptr;
	}
	
	//-----------------------------------------------------------------------------------
	
	template<class T, typename ...ArgTypes>
	inline std::shared_ptr<T> DataManager::RequestResourceByType(HashString inKey, ArgTypes&& ...args)
	{
		HashString className = Class::Get<T>().GetName();
		{
			std::shared_ptr<T> resource = std::dynamic_pointer_cast<T>(GetResource(inKey, m_resourcesMap[className]));
			if (resource)
			{
				return resource;
			}
		}
		std::shared_ptr<T> resource = ObjectBase::NewObject<T>(inKey, std::forward<ArgTypes>(args)...);
		if (resource)
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

