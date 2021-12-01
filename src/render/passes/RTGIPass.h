#ifndef __RT_GI_PASS_H__
#define __RT_GI_PASS_H__

#include <vulkan/vulkan.hpp>

#include "messages/MessageSubscriber.h"
#include "RenderPassBase.h"

namespace CGE
{

	class RTGIPass : public RenderPassBase
	{
	public:
		RTGIPass(HashString);
		~RTGIPass();
	protected:
		void ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable) override;
		void InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext) override;
	};

}

#endif
