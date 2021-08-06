#include "ClusteringManager.h"
#include "Renderer.h"
#include "core/Engine.h"

namespace CGE
{

	ClusteringManager::ClusteringManager()
	{
		m_subscriber.AddHandler<GlobalPreUpdateMessage>(this, &ClusteringManager::HandleUpdateMsg);
	}

	void ClusteringManager::Update()
	{
		Renderer* rend = Engine::GetRendererInstance();
		
		glm::ivec2 halfScreenSize(rend->GetWidth() / 2, rend->GetHeight() / 2);

		for (uint8_t idx = 0; idx < 2; idx++)
		{
			m_clusterSize[idx] = halfScreenSize[idx] / (m_maxNumClusters[idx] / 2);
			m_clusterSize[idx] += ((m_clusterSize[idx] * (m_maxNumClusters[idx] / 2)) < halfScreenSize[idx]) ? 1 : 0;
			m_clusterSize[idx] = ((m_clusterSize[idx] + m_supportedImageScaling - 1) / m_supportedImageScaling) * m_supportedImageScaling;
			m_numClusters[idx] = (halfScreenSize[idx] + m_clusterSize[idx] - 1) / m_clusterSize[idx];
			m_halfScreenOffset[idx] = m_clusterSize[idx] * m_numClusters[idx] - halfScreenSize[idx];
			m_clusterScreenOverflow[idx] = float(m_clusterSize[idx] * m_numClusters[idx]) / float(halfScreenSize[idx]);
			m_numClusters[idx] *= 2;
		}
	}

	void ClusteringManager::HandleUpdateMsg(std::shared_ptr<GlobalPreUpdateMessage> msg)
	{
		Update();
	}

}


