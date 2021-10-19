#include "DepthPrepass.h"
#include "core/Engine.h"
#include "../Renderer.h"
#include "../objects/VulkanDevice.h"
#include "utils/ResourceUtils.h"

namespace CGE
{

	void DepthPrepass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		
	}

	void DepthPrepass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		VulkanDevice& device = Engine::GetRendererInstance()->GetVulkanDevice();

		auto passData = std::make_shared<DepthPrepassData>();
		dataTable.AddPassData<DepthPrepassData>(passData);

		passData->depthTextures = ResourceUtils::CreateDepthAttachmentTextures(2, &device, initContext.GetWidth(), initContext.GetHeight());

		initContext.SetDepthAttachments(passData->depthTextures);
	}

}



