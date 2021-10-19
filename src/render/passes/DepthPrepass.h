#ifndef __DEPTH_PREPASS_H__
#define __DEPTH_PREPASS_H__

#include "RenderPassBase.h"
#include "utils/Identifiable.h"

namespace CGE
{

	//----------------------------------------------------------------------

	struct DepthPrepassData : public Identifiable<DepthPrepassData>
	{
		std::vector<Texture2DPtr> depthTextures;
	};

	//----------------------------------------------------------------------

	class DepthPrepass : public RenderPassBase
	{
	public:
		DepthPrepass() : RenderPassBase("DepthPrepass") {}
	protected:
		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;
	};

	//----------------------------------------------------------------------

}

#endif


