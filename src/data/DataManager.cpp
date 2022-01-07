
#include "data/DataManager.h"
#include "core/Class.h"
#include <assert.h>
#include <random>
#include <chrono>
#include "async/Job.h"
#include "async/ThreadPool.h"

namespace
{
	static constexpr uint32_t SAMPLE_COUNT = 5;
	static constexpr uint32_t SEQUENCE_SIZE = 10;
}

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
	
	DataManager* DataManager::m_instance = nullptr;
	std::mutex DataManager::m_staticMutex;
	
	DataManager::DataManager()
	{
		m_resourcesTable.reserve(1024 * 128);
		m_resourcesMap.reserve(128);

		m_messageSubscriber.AddHandler<GlobalPostFrameMessage>(this, &DataManager::HandleUpdate);
	}
	
	DataManager::~DataManager()
	{
	
	}
	
	DataManager* DataManager::GetInstance()
	{
		if (!m_instance)
		{
			std::scoped_lock<std::mutex> lock(m_staticMutex);
			if (!m_instance)
			{
				m_instance = new DataManager();
			}
		}
		return m_instance;
	}
	
	void DataManager::ShutdownInstance()
	{
		if (m_instance)
		{
			std::scoped_lock<std::mutex> lock(m_staticMutex);
			if (m_instance)
			{
				m_instance->CleanupResources();
				delete m_instance;
				m_instance = nullptr;
			}
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
		for (auto& chain : m_cleanupChain)
		{
			for (auto resPtr : chain)
			{
				resPtr->Destroy();
			}
			chain.clear();
		}
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
			HashString className = inValue->GetClass().GetName();
			if (m_resourcesMap.find(className) == m_resourcesMap.end())
			{
				m_resourcesMap[className].reserve(1024 * 16);
			}
			m_resourcesMap[className][key] = inValue;

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

	void DataManager::DestroyHint(HashString id)
	{
		std::scoped_lock<std::mutex> lock(m_mutex);

		m_deletionHints.push_back(id);
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

	void DataManager::HandleUpdate(std::shared_ptr<GlobalPostFrameMessage> updateMsg)
	{
		std::function func = [this]() { ScanForAbandonedResources(); };
		std::shared_ptr<Job<void()>> job = std::make_shared<Job<void()>>(std::move(func));
		ThreadPool::GetInstance()->AddJob(job);
		//ScanForAbandonedResources();
	}

	void DataManager::ScanForAbandonedResources()
	{
		{
			// process deletion hints first
			std::scoped_lock<std::mutex> lock(m_mutex);
			for (auto& id : m_deletionHints)
			{
				auto it = m_resourcesTable.find(id);
				if ((it != m_resourcesTable.end()) && (it->second.use_count() <= 2))
				{
					m_cleanupChain[m_cleanupChainIndex].push_back(it->second);
					m_resourcesMap[it->second->GetClass().GetName()].erase(id);
					m_resourcesTable.erase(id);
				}
			}
			m_deletionHints.clear();
		}

		std::random_device rd;
		std::mt19937 rng(static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count()));
		// sample resource container randomly looking for abandoned resources
		for (uint32_t idx = 0; idx < SAMPLE_COUNT; ++idx)
		{
			std::scoped_lock<std::mutex> lock(m_mutex);

			std::uniform_int_distribution<uint32_t> dist(0, static_cast<uint32_t>(m_resourcesTable.size()) - 1);

			auto it = m_resourcesTable.begin();
			uint32_t offset = dist(rng);
			std::advance(it, offset);

			auto endIt = m_resourcesTable.end();
			uint16_t counter = 0;
			while (it != endIt)
			{
				if (it->second.use_count() <= 2)
				{
					m_cleanupChain[m_cleanupChainIndex].push_back(it->second);
					m_resourcesMap[it->second->GetClass().GetName()].erase(it->first);
					it = m_resourcesTable.erase(it);
				}
				else
				{
					++it;
				}
				if (counter++ >= SEQUENCE_SIZE)
				{
					break;
				}
			}
		}

		m_cleanupChainIndex = (m_cleanupChainIndex + 1) % m_cleanupChain.size();
		m_cleanupChain[m_cleanupChainIndex].clear();
	}

}


