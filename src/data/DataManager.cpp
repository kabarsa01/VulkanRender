
#include "data/DataManager.h"
#include "core/Class.h"
#include <assert.h>

namespace CGE
{
	
	//---------------------------------------------------------------
	// using declarations and aliases
	//---------------------------------------------------------------
	
	template<typename T1, typename T2>
	using unordered_map = std::unordered_map<T1, T2>;
	
	template<typename T>
	using shared_ptr = std::shared_ptr<T>;
	
	//---------------------------------------------------------------
	
	DataManager* DataManager::m_instance = new DataManager();
	
	DataManager::DataManager()
	{
	
	}
	
	DataManager::~DataManager()
	{
	
	}
	
	DataManager* DataManager::GetInstance()
	{
		return m_instance;
	}
	
	void DataManager::ShutdownInstance()
	{
		m_instance->CleanupResources();
		if (m_instance != nullptr)
		{
			delete m_instance;
		}
	}
	
	void DataManager::CleanupResources()
	{
		std::scoped_lock<std::mutex> lock(m_mutex);
		auto it = m_resourcesTable.begin();
		for (; it != m_resourcesTable.end(); it++)
		{
			it->second->Destroy();
		}
		m_resourcesTable.clear();
		m_resourcesMap.clear();
	}

	bool DataManager::HasResource(HashString inKey)
	{
		std::scoped_lock<std::mutex> lock(m_mutex);
		return m_resourcesTable.find(inKey) != m_resourcesTable.end();
	}
	
	bool DataManager::AddResource(ResourcePtr inValue)
	{
		if (!inValue)
		{
			return false;
		}

		std::scoped_lock<std::mutex> lock(m_mutex);

		HashString key = inValue->GetResourceId();
		if (m_resourcesTable.find(key) == m_resourcesTable.end())
		{
			m_resourcesTable[key] = inValue;
			m_resourcesMap[inValue->GetClass().GetName()][key] = inValue;
			return true;
		}

		assert(false);
		return false;
	}
	
	std::shared_ptr<Resource> DataManager::GetResource(HashString inKey)
	{
		std::scoped_lock<std::mutex> lock(m_mutex);
		auto it = m_resourcesTable.find(inKey);
		if (it != m_resourcesTable.end())
		{
			return it->second;
		}
		return nullptr;
	}
	
	ResourcePtr DataManager::GetResource(HashString inKey, std::unordered_map<HashString, ResourcePtr>& inMap)
	{
		std::scoped_lock<std::mutex> lock(m_mutex);
		auto it = inMap.find(inKey);
		if (it != inMap.end())
		{
			return it->second;
		}
		return nullptr;
	}

	bool DataManager::DeleteResource(ResourcePtr inValue)
	{
		if (!inValue)
		{
			return false;
		}

		return DeleteResource(inValue->GetResourceId());
	}	
	
	bool DataManager::DeleteResource(HashString key)
	{
		std::scoped_lock<std::mutex> lock(m_mutex);

		auto it = m_resourcesTable.find(key);
		if (it != m_resourcesTable.end())
		{
			m_resourcesMap[it->second->GetClass().GetName()].erase(key);
			m_resourcesTable.erase(key);
			return true;
		}

		return false;
	}

}


