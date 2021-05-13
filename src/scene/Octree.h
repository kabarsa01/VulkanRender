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
		~Octree() {}

		void SetNodeMinSize(float nodeMinSize) { m_nodeMinSize = nodeMinSize; }

		inline void AddObject(T object);
		inline void Update();

		template<typename QueryObj, typename Output>
		inline void Query(const QueryObj& queryObj, std::function<QueryCompareFunc<QueryObj>> func, Output& output);
	private:
		std::deque<T> m_objects;
		OctreeNode<T>* m_rootNode;
		std::function<CompareFunc> m_compareFunc;
		ObjectPool<OctreeNode<T>> m_nodePool;
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
		void ThreadQueryNode(std::shared_ptr<std::promise<void>> promise, 
			const QueryObj& queryObj,
			std::function<QueryCompareFunc<QueryObj>> func,
			Output& output);
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

		for (uint8_t idx = 0; idx < 16; idx++)
		{
			auto promise = std::make_shared<std::promise<void>>();
			m_futures.emplace_back(promise->get_future());
			std::function<void()> processFunc = [this, promise, &queryObj, func, &output]()
			{
				ThreadQueryNode(promise, queryObj, func, output);
			};
			ThreadPool::GetInstance()->AddJob(CreateJobPtr<void()>(std::move(processFunc)));
		}

		for (std::future<void>& f : m_futures)
		{
			f.wait();
		}
		m_futures.clear();
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
	void Octree<T>::ThreadQueryNode(std::shared_ptr<std::promise<void>> promise,
		const QueryObj& queryObj,
		std::function<QueryCompareFunc<QueryObj>> func,
		Output& output)
	{
		OctreeNode<T>* currentNodes = nullptr;
		static bool waitForProc = false;
		bool currentThreadFlag = false;

		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(m_nodeListMutex);

				if (m_nodeProcessingList.size() == 0)
				{
					m_nodeListCondition.wait(lock,
					[this]() -> bool {
						if (waitForProc) return false;
						if (m_nodesProcessed < m_nodesTotal)
						{
							return m_nodeProcessingList.size() > 0;
						}
						return true;
					});
					if (m_nodesProcessed >= m_nodesTotal && m_nodeProcessingList.size() == 0)
					{
						promise->set_value();
						m_nodeListCondition.notify_all();
						break;
					}
				}

				currentNodes = m_nodeProcessingList.front();
				m_nodeProcessingList.pop_front();
				m_nodesTotal.fetch_add(8);
				if (m_nodeProcessingList.empty())
				{
					waitForProc = true;
					currentThreadFlag = true;
				}
			}

			for (uint8_t idx = 0; idx < 8; idx++)
			{
				OctreeNode<T>* node = reinterpret_cast<OctreeNode<T>*>(currentNodes + idx);
				if (func(queryObj, node))
				{
					{
						std::scoped_lock<std::mutex> lock(m_nodeListMutex);
						if (node->children)
						{
							m_nodeProcessingList.push_back(node->children);
							m_nodeListCondition.notify_one();
						}
						else
						{
							m_nodesTotal.fetch_sub(1);
						}
						if (waitForProc && currentThreadFlag)
						{
							waitForProc = false;
							currentThreadFlag = false;
						};
					}
					m_queryOutputMutex.lock();
					node->payload->WriteOutput(output);
					m_queryOutputMutex.unlock();
				}
				else
				{
					m_nodesTotal.fetch_sub(1);
				}
			}
			m_nodesProcessed.fetch_add(1);
		}
	}

	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

}

#endif

