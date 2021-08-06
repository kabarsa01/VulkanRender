#ifndef __CLUSTERING_MANAGER_H__
#define __CLUSTERING_MANAGER_H__

#include "messages/MessageSubscriber.h"
#include "glm/glm.hpp"

namespace CGE
{

	class ClusteringManager
	{
	public:
		ClusteringManager();
		~ClusteringManager() {}

		void SetSupportedImageScaling(uint8_t supportedImageScaling) { m_supportedImageScaling = supportedImageScaling; }
		void SetMaxNumClusters(glm::uvec2 maxNumClusters) { m_maxNumClusters = maxNumClusters; }

		glm::ivec2 GetClusterSize() { return m_clusterSize; }
		glm::ivec2 GetNumClusters() { return m_numClusters; }
		glm::ivec2 GetMaxNumClusters() { return m_maxNumClusters; }
		glm::ivec2 GetHalfScreenOffset() { return m_halfScreenOffset; }
		glm::vec2 GetClusterScreenOverflow() { return m_clusterScreenOverflow; }
	private:
		MessageSubscriber m_subscriber;

		uint8_t m_supportedImageScaling = 4;
		glm::ivec2 m_clusterSize;
		glm::ivec2 m_numClusters;
		glm::ivec2 m_maxNumClusters;
		glm::ivec2 m_halfScreenOffset;
		glm::vec2 m_clusterScreenOverflow;

		void Update();
		void HandleUpdateMsg(std::shared_ptr<GlobalPreUpdateMessage> msg);
	};

}

#endif

