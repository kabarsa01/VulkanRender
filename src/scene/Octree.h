#ifndef _OCTREE_H_
#define _OCTREE_H_

#include <list>
#include <vector>
#include <mutex>
#include <functional>

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

namespace CGE
{

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

		uint8_t CalculatePointSubnodeIndex(const glm::vec3& point)
		{
			glm::vec3 middlePoint = position + (size * 0.5f);

			uint8_t xPart = 4 * (point.x > middlePoint.x);
			uint8_t yPart = 2 * (point.y > middlePoint.y);
			uint8_t zPart = 1 * (point.z > middlePoint.z);

			return xPart + yPart + zPart;
		}
	};

	template<typename T>
	class Octree
	{
	public:
		using CompareFunc = uint8_t(T, OctreeNode<T>*);

		Octree(std::function<CompareFunc>&& compareFunc)
			: m_compareFunc(compareFunc)
		{
			m_rootNode = m_nodePool.Acquire(1);
			m_rootNode->position = glm::vec3(-300.0f, -300.0f, -300.0f);
			m_rootNode->size = glm::vec3(600.0f, 600.0f, 600.0f);
		}
		~Octree() {}

		void AddObject(T object)
		{
			m_objects.push_back(object);
		}

		void Update()
		{
			while (m_objects.size() > 0)
			{
				T obj = m_objects.front();
				m_objects.pop_front();

				m_rootNode->objects.push_back(obj);
			}

			std::list<OctreeNode<T>*> nextNodes;
			OctreeNode<T>* nodes = UpdateNode(m_rootNode);
			if (nodes != nullptr)
			{
				nextNodes.push_back(nodes);
			}
			while (nextNodes.size())
			{
				OctreeNode<T>* currentNodes = nextNodes.front();
				nextNodes.pop_front();

				for (uint8_t idx = 0; idx < 8; idx++)
				{
					nodes = UpdateNode(currentNodes + idx);
					if (nodes != nullptr)
					{
						nextNodes.push_back(nodes);
					}
				}
			}
		}
	private:
		// 32 MB for the scene tree
		ObjectPool<OctreeNode<T>, (1024*1024*32) / sizeof(OctreeNode<T>)> m_nodePool;

		std::deque<T> m_objects;
		OctreeNode<T>* m_rootNode;
		std::function<CompareFunc> m_compareFunc;
		float nodeMinSize = 1.0f;

		OctreeNode<T>* UpdateNode(OctreeNode<T>* node)
		{
			if (node->objects.size() <= 1)
			{
				return nullptr;
			}
			glm::vec3& nodeSize = node->size;
			if (nodeSize.x <= nodeMinSize || nodeSize.y <= nodeMinSize || nodeSize.z <= nodeMinSize)
			{
				return nullptr;
			}

			if (node->children == nullptr)
			{
				node->children = m_nodePool.Acquire(8);
				if (node->children == nullptr)
				{
					return nullptr;
				}

				glm::vec3 childSize = node->size * 0.5f;
				for (uint8_t idx = 0; idx < 8; idx++)
				{
					node->children[idx].parent = node;
					node->children[idx].size = childSize;

					float X = node->position.x + ((idx / 4) * childSize.x);
					float Y = node->position.y + (((idx % 4) / 2) * childSize.y);
					float Z = node->position.z + (((idx % 4) % 2) * childSize.z);

					node->children[idx].position = {X, Y, Z};
				}
			}

			for (T object : node->objects)
			{
				uint8_t cell = m_compareFunc(object, node);
				if (cell < 8)
				{
					node->children[cell].objects.push_back(object);
				}
			}

			return node->children;
		}
	};

}

#endif

