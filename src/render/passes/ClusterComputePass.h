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
		std::vector<MaterialPtr> computeMaterials;
	};

	class ClusterComputePass : public RenderPassBase
	{
	public:
		//Texture2DPtr texture;
		//Texture2DPtr depthTexture;

		ClusterComputePass();
		~ClusterComputePass();
	protected:
		MessageSubscriber m_subscriber;

		LightsList* m_lightsList;
		LightsIndices* m_lightsIndices;
		std::vector<MaterialPtr> m_computeMaterials;

		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;

		void HandleUpdate(const std::shared_ptr<GlobalPostSceneMessage> msg);
	};

}

#endif
