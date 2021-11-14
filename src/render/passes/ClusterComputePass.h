#ifndef __CLUSTER_COMPUTE_PASS_H__
#define __CLUSTER_COMPUTE_PASS_H__

#include "RenderPassBase.h"
#include "../DataStructures.h"
#include "data/Material.h"
#include "messages/MessageSubscriber.h"

namespace CGE
{

	class ClusterComputePass : public RenderPassBase
	{
	public:
		MaterialPtr computeMaterial;
		Texture2DPtr texture;
		Texture2DPtr depthTexture;

		ClusterComputePass();
		~ClusterComputePass();
	protected:
		MessageSubscriber m_subscriber;

		ClusterLightsData* clusterLightData;
		LightsList* lightsList;
		LightsIndices* lightsIndices;

		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;

		void HandleUpdate(const std::shared_ptr<GlobalPostSceneMessage> msg);
	};

}

#endif
