#ifndef __LIGHT_PROPAGATION_COMPUTE_PASS_H__
#define __LIGHT_PROPAGATION_COMPUTE_PASS_H__

#include "RenderPassBase.h"

namespace CGE
{

	class LightPropagationComputePass : public RenderPassBase
	{
	public:
		LightPropagationComputePass(HashString name);
		~LightPropagationComputePass();
	protected:
		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;
	private:
		MaterialPtr m_computeMaterial;
	};

}

#endif

