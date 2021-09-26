#include "RenderPassBase.h"
#include "utils/Singleton.h"
#include "../resources/VulkanBuffer.h"
#include "data/Texture2D.h"
#include "core/Engine.h"
#include "../Renderer.h"

namespace CGE
{

	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------

	void RenderPassBase::Init()
	{
		PassInitContext initContext;
		InitPass(*Singleton<RenderPassDataTable>::GetInstance(), initContext);

		m_device = &Engine::GetRendererInstance()->GetVulkanDevice();

		m_renderPass = CreateRenderPass(initContext);
		m_framebuffers = CreateFramebuffers(initContext);
	}

	//-----------------------------------------------------------------------------------------------------------

	void RenderPassBase::Execute(vk::CommandBuffer* commandBuffer)
	{
		PassExecuteContext executeContext(this);
		ExecutePass(commandBuffer, executeContext, *Singleton<RenderPassDataTable>::GetInstance());
	}

	//-----------------------------------------------------------------------------------------------------------

	vk::RenderPass RenderPassBase::CreateRenderPass(const PassInitContext& initContext)
	{
		std::vector<vk::AttachmentDescription> attachDescArray;
		std::vector<vk::AttachmentReference> colorAttachRefArray;

		attachDescArray.reserve(initContext.m_attachments.size());

		for (auto& attachRecPair : initContext.m_attachments)
		{
			Texture2DPtr texture = attachRecPair.second[0];
			vk::ImageCreateInfo& info = texture->GetImage().createInfo;

			vk::AttachmentDescription attachDesc;
			attachDesc.setFormat(info.format);
			attachDesc.setSamples(info.samples);
			attachDesc.setLoadOp(vk::AttachmentLoadOp::eClear);
			attachDesc.setStoreOp(vk::AttachmentStoreOp::eStore);
			attachDesc.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
			attachDesc.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
			attachDesc.setInitialLayout(vk::ImageLayout::eUndefined);
			attachDesc.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

			attachDescArray[attachRecPair.first] = attachDesc;

			vk::AttachmentReference attachRef;
			attachRef.setAttachment(attachRecPair.first);
			attachRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

			colorAttachRefArray.emplace_back(attachRef);
		}

		vk::SubpassDescription subpassDesc;
		subpassDesc.setColorAttachments(colorAttachRefArray);
		subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

		vk::AttachmentReference depthAttachRef;
		if (!initContext.m_depthAttachments.empty())
		{
			vk::ImageCreateInfo& info = initContext.m_depthAttachments[0]->GetImage().createInfo;

			vk::AttachmentDescription depthAttachDesc;
			depthAttachDesc.setFormat(info.format);
			depthAttachDesc.setSamples(info.samples);
			depthAttachDesc.setLoadOp(vk::AttachmentLoadOp::eLoad);
			depthAttachDesc.setStoreOp(vk::AttachmentStoreOp::eStore);
			depthAttachDesc.setStencilLoadOp(vk::AttachmentLoadOp::eLoad);
			depthAttachDesc.setStencilStoreOp(vk::AttachmentStoreOp::eStore);
			depthAttachDesc.setInitialLayout(ImageLayout::eUndefined);
			depthAttachDesc.setFinalLayout(ImageLayout::eDepthStencilAttachmentOptimal);

			depthAttachRef.setAttachment(attachDescArray.size());
			depthAttachRef.setLayout(ImageLayout::eDepthStencilAttachmentOptimal);

			attachDescArray.emplace_back(depthAttachDesc);
			// set depth attachment to subpass
			subpassDesc.setPDepthStencilAttachment(&depthAttachRef);
		}

		vk::SubpassDependency subpassDependency;
		subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
		subpassDependency.setDstSubpass(0);
		subpassDependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		subpassDependency.setSrcAccessMask(vk::AccessFlags());
		subpassDependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		subpassDependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

		vk::RenderPassCreateInfo passCreateInfo;
		passCreateInfo.setAttachments(attachDescArray);
		passCreateInfo.setSubpassCount(1);
		passCreateInfo.setPSubpasses(&subpassDesc);
		passCreateInfo.setDependencyCount(1);
		passCreateInfo.setPDependencies(&subpassDependency);

		return m_device->GetDevice().createRenderPass(passCreateInfo);
	}

	//-----------------------------------------------------------------------------------------------------------

	std::vector<vk::Framebuffer> RenderPassBase::CreateFramebuffers(const PassInitContext& initContext)
	{
		std::vector<vk::Framebuffer> framebuffers;

		uint32_t framebufferCount = 0;
		for (auto& pair : initContext.m_attachments)
		{
			if (pair.second.size() > framebufferCount)
			{
				framebufferCount = pair.second.size();
			}
		}

		for (uint32_t framebufferIndex = 0; framebufferIndex < framebufferCount; ++framebufferIndex)
		{
			std::vector<vk::ImageView> viewArray;
			viewArray.reserve(initContext.m_attachments.size());

			for (auto& attachPair : initContext.m_attachments)
			{
				uint32_t index = framebufferIndex % attachPair.second.size();
				viewArray[attachPair.first] = attachPair.second[index]->GetImageView();
			}
			if (!initContext.m_depthAttachments.empty())
			{
				uint32_t index = framebufferIndex % initContext.m_depthAttachments.size();
				viewArray.push_back(initContext.m_depthAttachments[index]->GetImageView());
			}

			vk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.setRenderPass(m_renderPass);
			framebufferInfo.setAttachments(viewArray);
			framebufferInfo.setWidth(m_width);
			framebufferInfo.setHeight(m_height);
			framebufferInfo.setLayers(1);

			framebuffers.emplace_back(m_device->GetDevice().createFramebuffer(framebufferInfo));
		}

		return framebuffers;
	}

	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------

	vk::RenderPass PassExecuteContext::GetRenderPass()
	{
		return m_owner->m_renderPass;
	}

	vk::Framebuffer PassExecuteContext::GetFramebuffer(uint32_t frameIndex /*= UINT32_MAX*/)
	{
		if (frameIndex == UINT32_MAX)
		{
			frameIndex = Engine::Get()->GetFrameCount() % m_owner->m_framebuffers.size();
		}

		return m_owner->m_framebuffers[frameIndex];
	}

}

