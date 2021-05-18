#ifndef _OCTREE_H_
#define _OCTREE_H_

#include <list>
#include <vector>
#include <mutex>
#include <atomic>
#include <future>
#include <functional>

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

namespace CGE
{

	//=============================================================================================
	// OCTREE NODE PAYLOAD
	//=============================================================================================
	//=============================================================================================
	//
	//=============================================================================================

	template<typename T>
	struct OctreeNodePayload
	{
		void Add(T&) {}
		void Remove(T&) {}
		template<class Output>
		void WriteOutput(Output&) {}
		bool IsEmpty() { return true; }
	};

	//=============================================================================================
	//
	//=============================================================================================
	//=============================================================================================
	// OCTREE NODE
	//=============================================================================================

	template<typename T>
	class OctreeNode
	{
	public:
		OctreeNode<T>* parent;
		OctreeNode<T>* children;

		glm::vec3 position;
		glm::vec3 size;

		std::list<T> objects;
		OctreeNodePayload<T>* payload;
		std::mutex mutex;

		OctreeNode()
			: parent(nullptr)
			, children(nullptr)
			, position {0.0f, 0.0f, 0.0f}
			, size { 0.0f, 0.0f, 0.0f }
		{
			payload = new OctreeNodePayload<T>();
		}

		OctreeNode(glm::vec3 inPosition, glm::vec3 inSize)
			: parent(nullptr)
			, children(nullptr)
			, position(inPosition)
			, size(inSize)
		{
			payload = new OctreeNodePayload<T>();
		}

		~OctreeNode() { delete payload; }

		void Lock()
		{
			mutex.lock();
		}

		void Unlock()
		{
			mutex.unlock();
		}

		uint8_t CalculatePointSubnodeIndex(const glm::vec3& point)
		{
			glm::vec3 middlePoint = position + (size * 0.5f);

			uint8_t xPart = 4 * (point.x > middlePoint.x);
			uint8_t yPart = 2 * (point.y > middlePoint.y);
			uint8_t zPart = 1 * (point.z > middlePoint.z);

			return xPart + yPart + zPart;
		}

		OctreeNode<T>* CreateChildren(ObjectPool<OctreeNode<T>>& nodePool, bool lock)
		{
			OctreeNode<T>* nodes = nullptr;
			if (nullptr == children)
			{
				nodes = nodePool.Acquire(8);
				if (nodes == nullptr)
				{
					return nullptr;
				}

				glm::vec3 childSize = size * 0.5f;
				for (uint8_t idx = 0; idx < 8; idx++)
				{
					nodes[idx].parent = this;
					nodes[idx].size = childSize;

					float X = position.x + ((idx / 4) * childSize.x);
					float Y = position.y + (((idx % 4) / 2) * childSize.y);
					float Z = position.z + (((idx % 4) % 2) * childSize.z);

					nodes[idx].position = { X, Y, Z };
				}
			}
			else
			{
				nodes = children;
			}

			if (lock)
			{	
				for (uint8_t idx = 0; idx < 8; idx++)
				{
					nodes[idx].Lock();
				}
			}

			children = nodes;

			return children;
		}

	};

	//=============================================================================================
	//=============================================================================================
	// OCTREE
	//=============================================================================================
	//=============================================================================================

	template<typename T>
	class Octree
	{
	public:
		using CompareFunc = uint8_t(T, OctreeNode<T>*);
		template<typename QueryObj>
		using QueryCompareFunc = bool(const QueryObj&, OctreeNode<T>*);

		Octree(uint32_t nodePoolSize, std::function<CompareFunc>&& compareFunc);
		~Octree();

		void SetNodeMinSize(float nodeMinSize) { m_nodeMinSize = nodeMinSize; }

		inline void AddObject(T object);
		inline void Update();
		inline OctreeNode<T>** GetNodeQueryResults() { return m_nodeResults; }
		inline uint32_t GetNodeQueryResultCount() { return m_nodeResultCounter; }

		template<typename QueryObj, typename Output>
		inline void Query(const QueryObj& queryObj, std::function<QueryCompareFunc<QueryObj>> func, Output& output);
	private:
		std::deque<T> m_objects;
		OctreeNode<T>* m_rootNode;
		std::function<CompareFunc> m_compareFunc;
		ObjectPool<OctreeNode<T>> m_nodePool;
		OctreeNode<T>** m_nodeRecords;
		OctreeNode<T>** m_nodeResults;
		std::promise<void> m_nodeProcessingPromise;
		std::atomic<uint32_t> m_nodeReserveCounter;
		std::atomic<uint32_t> m_nodeAddedCounter;
		std::atomic<uint32_t> m_nodeProcessingCounter;
		std::atomic<uint32_t> m_nodeProcessedCounter;
		std::atomic<uint32_t> m_nodeResultCounter;

		float m_nodeMinSize = 1.0f;

		std::list<OctreeNode<T>*> m_nodeProcessingList;
		std::mutex m_nodeListMutex;
		std::mutex m_queryOutputMutex;
		std::condition_variable m_nodeListCondition;
		std::atomic<uint32_t> m_nodesTotal;
		std::atomic<uint32_t> m_nodesProcessed;
		std::vector<std::future<void>> m_futures;

		OctreeNode<T>* UpdateNode(OctreeNode<T>* node);

		void ThreadUpdateNode(std::shared_ptr<std::promise<void>> promise);
		template<typename QueryObj, typename Output>
		void ThreadQueryNode(const QueryObj& queryObj, std::function<QueryCompareFunc<QueryObj>> func, Output& output);
	};

	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

	template<typename T>
	Octree<T>::Octree(uint32_t nodePoolSize, std::function<CompareFunc>&& compareFunc)
		: m_compareFunc(compareFunc)
		, m_nodePool(nodePoolSize)
	{
		m_rootNode = m_nodePool.Acquire(1);
		m_rootNode->position = glm::vec3(-1000.0f, -1000.0f, -1000.0f);
		m_rootNode->size = glm::vec3(2000.0f, 2000.0f, 2000.0f);

		m_nodeRecords = new OctreeNode<T>*[nodePoolSize];
		m_nodeResults = new OctreeNode<T>*[nodePoolSize];
	}

	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

	template<typename T>
	CGE::Octree<T>::~Octree()
	{
		delete[] m_nodeRecords;
		delete[] m_nodeResults;
	}

	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

	template<typename T>
	void Octree<T>::AddObject(T object)
	{
		m_objects.push_back(object);
	}

	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

	template<typename T>
	void Octree<T>::Update()
	{
		while (m_objects.size() > 0)
		{
			T obj = m_objects.front();
			m_objects.pop_front();

			m_rootNode->objects.push_back(obj);
		}

		OctreeNode<T>* nodes = UpdateNode(m_rootNode);
		if (nodes == nullptr)
		{
			return;
		}
		m_nodeProcessingList.push_back(nodes);

		for (uint8_t idx = 0; idx < 16; idx++)
		{
			auto promise = std::make_shared<std::promise<void>>();
			m_futures.emplace_back(promise->get_future());
			ThreadPool::GetInstance()->AddJob(CreateJobPtr<void()>(std::bind(&Octree<T>::ThreadUpdateNode, this, promise)));
		}

		// TODO: make waiting somewhere in other place
		for (std::future<void>& f : m_futures)
		{
			f.wait();
		}
		m_futures.clear();
	}

	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

	template<typename T>
	template<typename QueryObj, typename Output>
	void Octree<T>::Query(const QueryObj& queryObj, std::function<QueryCompareFunc<QueryObj>> func, Output& output)
	{
		if (m_rootNode->children == nullptr || !func(queryObj, m_rootNode))
		{
			return;
		}

		m_nodeProcessingList.push_back(m_rootNode->children);

		m_nodesTotal.store(1);
		m_nodesProcessed.store(0);

		m_nodeResultCounter.store(1);
		m_nodeResults[0] = m_rootNode;

		if (m_rootNode->children)
		{
			m_nodeProcessingCounter.store(0);
			m_nodeProcessedCounter.store(0);
			m_nodeReserveCounter.store(1);
			m_nodeAddedCounter.store(1);
			m_nodeRecords[0] = m_rootNode->children;

			std::future<void> future = m_nodeProcessingPromise.get_future();
			std::function<void()> processFunc = [this, &queryObj, func, &output]()
			{
				ThreadQueryNode(queryObj, func, output);
			};
			ThreadPool::GetInstance()->AddJob(CreateJobPtr<void()>(std::move(processFunc)));
			future.wait();

			m_nodeProcessingPromise.~promise<void>();
			new(&m_nodeProcessingPromise) std::promise<void>();
		}

		for (uint32_t idx = 0; idx < m_nodeResultCounter; idx++)
		{
			m_nodeResults[idx]->payload->WriteOutput(output);
		}
	}

	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

	template<typename T>
	OctreeNode<T>* Octree<T>::UpdateNode(OctreeNode<T>* node)
	{
		std::scoped_lock<std::mutex> lock(node->mutex);

		bool leafNode = false;
		if (node->objects.size() <= 1)
		{
			leafNode = true;
		}
		glm::vec3& nodeSize = node->size;
		if (nodeSize.x <= m_nodeMinSize || nodeSize.y <= m_nodeMinSize || nodeSize.z <= m_nodeMinSize)
		{
			leafNode = true;
		}
		if (leafNode)
		{
			for (T& obj : node->objects)
			{
				node->payload->Add(obj);
			}
			node->objects.clear();
			return nullptr;
		}			


		if (!node->CreateChildren(m_nodePool, true))
		{
			return nullptr;
		}

		for (auto iter = node->objects.begin(); iter != node->objects.end(); iter++)
		{
			T object = *iter;
			uint8_t cell = m_compareFunc(object, node);
			if (cell < 8)
			{
				node->children[cell].objects.push_back(object);
			}
			else
			{
				node->payload->Add(object);
			}
		}
		node->objects.clear();
		for (uint8_t idx = 0; idx < 8; idx++)
		{
			node->children[idx].Unlock();
		}

		return node->children;
	}

	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

	template<typename T>
	void Octree<T>::ThreadUpdateNode(std::shared_ptr<std::promise<void>> promise)
	{
		uint32_t retryCounter = 0;

		while (true)
		{
			m_nodeListMutex.lock();

			if (m_nodeProcessingList.size() == 0)
			{
				m_nodeListMutex.unlock();
				if (retryCounter++ < 10)
				{
					std::this_thread::yield();
					continue;
				}
				promise->set_value();
				break;
			}

			OctreeNode<T>* currentNodes = m_nodeProcessingList.front();
			m_nodeProcessingList.pop_front();

			m_nodeListMutex.unlock();

			for (uint8_t idx = 0; idx < 8; idx++)
			{
				OctreeNode<T>* nodes = UpdateNode(currentNodes + idx);
				if (nodes != nullptr)
				{
					m_nodeListMutex.lock();
					m_nodeProcessingList.push_back(nodes);
					m_nodeListMutex.unlock();
				}
			}
		}
	}

	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

	template<typename T>
	template<typename QueryObj, typename Output>
	void Octree<T>::ThreadQueryNode(const QueryObj& queryObj, std::function<QueryCompareFunc<QueryObj>> func, Output& output)
	{
		if (m_nodeProcessedCounter > m_nodeAddedCounter)
		{
			return;
		}

		uint32_t nodeProcIndex = m_nodeProcessingCounter.fetch_add(1);
		if (nodeProcIndex > m_nodeAddedCounter - 1)
		{
			return;
		}
		OctreeNode<T>* currentNodes = m_nodeRecords[nodeProcIndex];

		for (uint8_t idx = 0; idx < 8; idx++)
		{
			OctreeNode<T>* node = reinterpret_cast<OctreeNode<T>*>(currentNodes + idx);
			if (func(queryObj, node))
			{
				if (node->children)
				{
					uint32_t nodeStoreIndex = m_nodeReserveCounter.fetch_add(1);
					m_nodeRecords[nodeStoreIndex] = node->children;
					m_nodeAddedCounter.fetch_add(1);

					std::function<void()> processFunc = [this, &queryObj, func, &output]()
					{
						ThreadQueryNode(queryObj, func, output);
					};
					ThreadPool::GetInstance()->AddJob(CreateJobPtr<void()>(std::move(processFunc)));

				}
				if (!node->payload->IsEmpty())
				{
					uint32_t nodeResult = m_nodeResultCounter.fetch_add(1);
					m_nodeResults[nodeResult] = node;
				}
			}
		}

		uint32_t nodeProcessed = m_nodeProcessedCounter.fetch_add(1);
		if (nodeProcessed >= m_nodeAddedCounter - 1)
		{
			m_nodeProcessingPromise.set_value();
		}
	}

	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

}

#endif

