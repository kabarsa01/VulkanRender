#pragma once

#include "vulkan/vulkan.hpp"
#include "RenderPassBase.h"

namespace CGE
{
	class PostProcessPass : public RenderPassBase
	{
	public:
		PostProcessPass(HashString inName);
	protected:
		std::vector<MaterialPtr> m_postProcessMaterials;

		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;
	private:
	};
}
