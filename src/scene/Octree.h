#ifndef _OCTREE_H_
#define _OCTREE_H_

#include <list>
#include <vector>
#include <mutex>
#include <future>
#include <functional>

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

namespace CGE
{

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
		std::mutex mutex;

		OctreeNode()
			: parent(nullptr)
			, children(nullptr)
			, position {0.0f, 0.0f, 0.0f}
			, size { 0.0f, 0.0f, 0.0f }
		{
		}

		OctreeNode(glm::vec3 inPosition, glm::vec3 inSize)
			: parent(nullptr)
			, children(nullptr)
			, position(inPosition)
			, size(inSize)
		{
		}

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

		Octree(uint32_t nodePoolSize, std::function<CompareFunc>&& compareFunc);
		~Octree() {}

		void SetNodeMinSize(float nodeMinSize) { m_nodeMinSize = nodeMinSize; }

		inline void AddObject(T object);
		inline void Update();
	private:
		std::deque<T> m_objects;
		OctreeNode<T>* m_rootNode;
		std::function<CompareFunc> m_compareFunc;
		ObjectPool<OctreeNode<T>> m_nodePool;
		float m_nodeMinSize = 1.0f;
		std::list<OctreeNode<T>*> m_nodeProcessingList;
		std::mutex m_nodeListMutex;
		std::vector<std::future<void>> m_futures;

		OctreeNode<T>* UpdateNode(OctreeNode<T>* node);

		void ThreadUpdateNode(std::shared_ptr<std::promise<void>> promise);
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
	OctreeNode<T>* Octree<T>::UpdateNode(OctreeNode<T>* node)
	{
		std::scoped_lock<std::mutex> lock(node->mutex);

		if (node->objects.size() <= 1)
		{
			return nullptr;
		}
		glm::vec3& nodeSize = node->size;
		if (nodeSize.x <= m_nodeMinSize || nodeSize.y <= m_nodeMinSize || nodeSize.z <= m_nodeMinSize)
		{
			return nullptr;
		}

		if (!node->CreateChildren(m_nodePool, true))
		{
			return nullptr;
		}

		for (auto iter = node->objects.begin(); iter != node->objects.end();)
		{
			T object = *iter;
			uint8_t cell = m_compareFunc(object, node);
			if (cell < 8)
			{
				node->children[cell].objects.push_back(object);
				iter = node->objects.erase(iter);
			}
			else
			{
				++iter;
			}
		}
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
				if (retryCounter++ < 100)
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

}

#endif

