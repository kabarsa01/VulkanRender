#ifndef _OCTREE_H_
#define _OCTREE_H_

#include <glm/glm.hpp>

namespace CGE
{

	class OctreeNode
	{
	public:
		// we will use pool indices instead of pointers to reduce node size
		uint32_t parent;
		uint32_t children[8];

		glm::vec3 position;
		glm::uvec3 size;
	};

	template<typename T>
	class Octree
	{
	public:
	private:
		ObjectPool<OctreeNode, (1024*1024*16) / sizeof(OctreeNode)> m_nodePool; // 16MB is around 280'000 nodes, should be enough i hope
	};

}

#endif
