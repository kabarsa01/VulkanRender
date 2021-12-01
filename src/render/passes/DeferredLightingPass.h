#pragma once

#include "RenderPassBase.h"
#include "data/Material.h"

namespace CGE
{

	struct DeferredLightingData : public Identifiable<DeferredLightingData>
	{
		std::vector<Texture2DPtr> hdrRenderTargets;
	};

	class DeferredLightingPass : public RenderPassBase
	{
	public:
		DeferredLightingPass(HashString inName);
	protected:
		std::vector<MaterialPtr> m_lightingMaterials;

		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;
	};
}
