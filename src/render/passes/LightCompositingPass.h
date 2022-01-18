#ifndef __LIGHT_COMPOSITING_PASS_H__
#define __LIGHT_COMPOSITING_PASS_H__

#include <vector>

#include "vulkan/vulkan.hpp"
#include "RenderPassBase.h"
#include "utils/Identifiable.h"
#include "data/Texture2D.h"
#include "data/Material.h"

namespace CGE
{

	struct LightCompositingPassData : Identifiable<LightCompositingPassData>
	{
		std::vector<Texture2DPtr> frameImages;
	};

	class LightCompositingPass : public RenderPassBase
	{
	public:
		LightCompositingPass(HashString name);
		~LightCompositingPass();
	protected:
		std::vector<MaterialPtr> m_materials;

		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;
	};

}

#endif
