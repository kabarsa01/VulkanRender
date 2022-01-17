#pragma once

#include "vulkan/vulkan.hpp"
#include <array>

#include "RenderPassBase.h"

namespace CGE
{

	struct GBufferPassData : public Identifiable<GBufferPassData>
	{
		std::vector<Texture2DPtr> albedos;
		std::vector<Texture2DPtr> normals;
		std::vector<Texture2DPtr> velocity;
	};

	class GBufferPass : public RenderPassBase
	{
	public:
		GBufferPass(HashString inName);
	protected:
		std::array<vk::ClearValue, 4> clearValues;
	
		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;

	};

}
