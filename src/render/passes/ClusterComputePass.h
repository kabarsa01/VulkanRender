#ifndef __CLUSTER_COMPUTE_PASS_H__
#define __CLUSTER_COMPUTE_PASS_H__

#include "RenderPassBase.h"
#include "../DataStructures.h"
#include "data/Material.h"
#include "messages/MessageSubscriber.h"
#include "utils/Identifiable.h"
#include <vector>

namespace CGE
{

	struct ClusterComputeData : public Identifiable<ClusterComputeData>
	{
		std::vector<BufferDataPtr> lightsList;
		std::vector<BufferDataPtr> lightsIndices;
		BufferDataPtr clusterLightsData;
		BufferDataPtr gridLightsData;
	};

	class ClusterComputePass : public RenderPassBase
	{
	public:
		ClusterComputePass(const HashString& name);
		~ClusterComputePass();
	protected:
		MessageSubscriber m_subscriber;

		LightsList* m_lightsList;
		LightsIndices* m_lightsIndices;
		std::vector<MaterialPtr> m_computeMaterials;
		std::vector<MaterialPtr> m_gridComputeMaterials;

		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;

		void HandleUpdate(const std::shared_ptr<GlobalPostSceneMessage> msg);
		BufferDataPtr CreateLightsGrid();
	};

}

#endif
