#include "RenderPassBase.h"
#include "utils/Singleton.h"
#include "../resources/VulkanBuffer.h"
#include "data/Texture2D.h"
#include "core/Engine.h"
#include "../Renderer.h"
#include "../PerFrameData.h"

namespace CGE
{

	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------

	RenderPassBase::RenderPassBase(HashString name)
		: m_name(name)
	{
		m_initContext = new PassInitContext(this);
		m_executeContext = new PassExecuteContext(this);
	}

	//-----------------------------------------------------------------------------------------------------------

	RenderPassBase::~RenderPassBase()
	{
		delete m_initContext;
		delete m_executeContext;

		vk::Device& device = Engine::GetRendererInstance()->GetDevice();

		for (vk::Framebuffer fb : m_framebuffers)
		{
			device.destroyFramebuffer(fb);
		}
		device.destroyRenderPass(m_renderPass);
	}

	//-----------------------------------------------------------------------------------------------------------

	void RenderPassBase::Init()
	{
		m_width = Engine::GetRendererInstance()->GetWidth();
		m_height = Engine::GetRendererInstance()->GetHeight();

		InitPass(*Singleton<RenderPassDataTable>::GetInstance(), *m_initContext);

		m_device = &Engine::GetRendererInstance()->GetVulkanDevice();

		m_renderPass = CreateRenderPass(*m_initContext);
		m_framebuffers = CreateFramebuffers(*m_initContext);

		m_executeContext->m_attachments = m_initContext->m_attachments;
		m_executeContext->m_depthAttachments = m_initContext->m_depthAttachments;
	}

	//-----------------------------------------------------------------------------------------------------------

	void RenderPassBase::Execute(vk::CommandBuffer* commandBuffer)
	{
		// barriers ----------------------------------------------
		std::vector<ImageMemoryBarrier> barriers;
		if (!m_executeContext->m_depthAttachments.empty())
		{
			ImageMemoryBarrier depthTextureBarrier = m_executeContext->GetDepthAttachment()->GetImage().CreateLayoutBarrier(
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal,
				vk::AccessFlagBits::eShaderRead,
				vk::AccessFlagBits::eShaderWrite,
				vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
				0, 1, 0, 1);
			barriers.emplace_back(depthTextureBarrier);
		}
		for (const Texture2DPtr& texture : m_executeContext->GetFrameAttachments())
		{
			ImageMemoryBarrier textureBarrier = texture->GetImage().CreateLayoutBarrier(
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::AccessFlagBits::eShaderRead,
				vk::AccessFlagBits::eShaderWrite,
				vk::ImageAspectFlagBits::eColor,
				0, 1, 0, 1);
			barriers.emplace_back(textureBarrier);
		}
		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::DependencyFlags(),
			0, nullptr, 0, nullptr,
			static_cast<uint32_t>(barriers.size()), barriers.data());

		ExecutePass(commandBuffer, *m_executeContext, *Singleton<RenderPassDataTable>::GetInstance());
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
			depthAttachDesc.setLoadOp(vk::AttachmentLoadOp::eClear);
			depthAttachDesc.setStoreOp(vk::AttachmentStoreOp::eStore);
			depthAttachDesc.setStencilLoadOp(vk::AttachmentLoadOp::eClear);
			depthAttachDesc.setStencilStoreOp(vk::AttachmentStoreOp::eStore);
			depthAttachDesc.setInitialLayout(ImageLayout::eUndefined);
			depthAttachDesc.setFinalLayout(ImageLayout::eDepthStencilAttachmentOptimal);

			depthAttachRef.setAttachment(static_cast<uint32_t>( attachDescArray.size() ));
			depthAttachRef.setLayout(ImageLayout::eDepthStencilAttachmentOptimal);

			attachDescArray.emplace_back(depthAttachDesc);
			// set depth attachment to subpass
			subpassDesc.setPDepthStencilAttachment(&depthAttachRef);
		}

		vk::SubpassDependency subpassDependency;
		subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
		subpassDependency.setDstSubpass(0);
		subpassDependency.setSrcStageMask(vk::PipelineStageFlagBits::eAllCommands);
		subpassDependency.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead);
		subpassDependency.setDstStageMask(vk::PipelineStageFlagBits::eAllCommands);
		subpassDependency.setDstAccessMask(vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead);

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

		uint32_t framebufferCount = static_cast<uint32_t>(initContext.m_depthAttachments.size());
		for (auto& pair : initContext.m_attachments)
		{
			if (pair.second.size() > framebufferCount)
			{
				framebufferCount = static_cast<uint32_t>(pair.second.size());
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

	vk::Pipeline RenderPassBase::CreateGraphicsPipeline(const PassInitContext& initContext, MaterialPtr material, vk::PipelineLayout layout)
	{
		std::vector<PipelineShaderStageCreateInfo> shaderStageInfoArray = { material->GetVertexStageInfo(), material->GetFragmentStageInfo() };

		VertexInputBindingDescription bindingDesc = MeshData::GetBindingDescription(0);
		std::array<VertexInputAttributeDescription, 5> attributeDesc = Vertex::GetAttributeDescriptions(0);
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
		vertexInputInfo.setVertexBindingDescriptionCount(1);
		vertexInputInfo.setPVertexBindingDescriptions(&bindingDesc);
		vertexInputInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDesc.size()));
		vertexInputInfo.setPVertexAttributeDescriptions(attributeDesc.data());

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList);
		inputAssemblyInfo.setPrimitiveRestartEnable(VK_FALSE);

		Viewport viewport;
		viewport.setX(0.0f);
		viewport.setY(0.0f);
		viewport.setWidth(static_cast<float>(m_width));
		viewport.setHeight(static_cast<float>(m_height));
		viewport.setMinDepth(0.0f);
		viewport.setMaxDepth(1.0f);

		vk::Rect2D scissor;
		scissor.setOffset(vk::Offset2D(0, 0));
		scissor.setExtent(vk::Extent2D{ m_width, m_height });

		vk::PipelineViewportStateCreateInfo viewportInfo;
		viewportInfo.setViewportCount(1);
		viewportInfo.setPViewports(&viewport);
		viewportInfo.setScissorCount(1);
		viewportInfo.setPScissors(&scissor);

		vk::PipelineMultisampleStateCreateInfo multisampleInfo;

		std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates;
		colorBlendAttachmentStates.reserve(initContext.m_attachments.size());
		for (auto attachPair : initContext.m_attachments)
		{
			vk::PipelineColorBlendAttachmentState& blendState = colorBlendAttachmentStates[attachPair.first];
			blendState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
			blendState.setBlendEnable(VK_FALSE);
			blendState.setSrcColorBlendFactor(vk::BlendFactor::eOne);
			blendState.setDstColorBlendFactor(vk::BlendFactor::eZero);
			blendState.setColorBlendOp(vk::BlendOp::eAdd);
			blendState.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
			blendState.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
			blendState.setAlphaBlendOp(vk::BlendOp::eAdd);
		}

		vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
		colorBlendInfo.setLogicOpEnable(VK_FALSE);
		colorBlendInfo.setLogicOp(vk::LogicOp::eCopy);
		colorBlendInfo.setAttachments(colorBlendAttachmentStates);
		colorBlendInfo.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.setStageCount(static_cast<uint32_t>( shaderStageInfoArray.size() ));
		pipelineInfo.setPStages(shaderStageInfoArray.data());
		pipelineInfo.setPVertexInputState(&vertexInputInfo);
		pipelineInfo.setPInputAssemblyState(&inputAssemblyInfo);
		pipelineInfo.setPViewportState(&viewportInfo);
		pipelineInfo.setPRasterizationState(&initContext.rasterizationInfo);
		pipelineInfo.setPMultisampleState(&multisampleInfo);
		pipelineInfo.setPDepthStencilState(initContext.m_depthAttachments.size() > 0 ? &initContext.depthInfo : nullptr);
		pipelineInfo.setPColorBlendState(&colorBlendInfo);
		pipelineInfo.setPDynamicState(nullptr);
		pipelineInfo.setLayout(layout);
		pipelineInfo.setRenderPass(m_renderPass);
		pipelineInfo.setSubpass(0);
		pipelineInfo.setBasePipelineHandle(vk::Pipeline());
		pipelineInfo.setBasePipelineIndex(-1);

		return m_device->GetDevice().createGraphicsPipeline(m_device->GetPipelineCache(), pipelineInfo).value;
	}

	vk::Pipeline RenderPassBase::CreateComputePipeline(const PassInitContext& initContext, MaterialPtr material, vk::PipelineLayout layout)
	{
		vk::ComputePipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.setLayout(layout);
		pipelineCreateInfo.setStage(material->GetComputeStageInfo());

		return m_device->GetDevice().createComputePipeline(m_device->GetPipelineCache(), pipelineCreateInfo).value;
	}

	vk::PipelineLayout RenderPassBase::CreatePipelineLayout(std::vector<DescriptorSetLayout>& descriptorSetLayouts)
	{
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setFlags(vk::PipelineLayoutCreateFlags());
		pipelineLayoutInfo.setSetLayouts(descriptorSetLayouts);

		vk::PushConstantRange pushConstRange;
		pushConstRange.setOffset(0);
		pushConstRange.setSize(sizeof(uint32_t));
		pushConstRange.setStageFlags(vk::ShaderStageFlagBits::eAll);

		pipelineLayoutInfo.setPushConstantRangeCount(1);
		pipelineLayoutInfo.setPPushConstantRanges(&pushConstRange);

		return m_device->GetDevice().createPipelineLayout(pipelineLayoutInfo);
	}

	PipelineData& RenderPassBase::CreateOrFindPipeline(const PassInitContext& initContext, MaterialPtr material)
	{
		Device& device = m_device->GetDevice();

		PipelineRegistry& pipelineRegistry = *PipelineRegistry::GetInstance();
		// check pipeline storage and create new pipeline in case it was not created before
		if (!pipelineRegistry.HasPipeline(m_name, material->GetShaderHash()))
		{
			PipelineData pipelineData;

			PerFrameData* frameData = Engine::GetRendererInstance()->GetPerFrameData();

			std::vector<vk::DescriptorSet> sets = material->GetDescriptorSets();
			sets[0] = frameData->GetSet();
			pipelineData.descriptorSets = sets;

			std::vector<vk::DescriptorSetLayout> layouts = material->GetDescriptorSetLayouts();
			layouts[0] = frameData->GetLayout();
			pipelineData.pipelineLayout = CreatePipelineLayout(layouts);
			if (initContext.compute)
			{
				pipelineData.pipeline = CreateComputePipeline(initContext, material, pipelineData.pipelineLayout);
			}
			else
			{
				pipelineData.pipeline = CreateGraphicsPipeline(initContext, material, pipelineData.pipelineLayout);
			}

			pipelineRegistry.StorePipeline(m_name, material->GetShaderHash(), pipelineData);
		}

		return pipelineRegistry.GetPipeline(m_name, material->GetShaderHash());
	}

	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------

	PassInitContext::PassInitContext(RenderPassBase* owner)
		: m_owner(owner)
	{
		rasterizationInfo.setDepthClampEnable(VK_FALSE);
		rasterizationInfo.setRasterizerDiscardEnable(VK_FALSE);
		rasterizationInfo.setPolygonMode(vk::PolygonMode::eFill);
		rasterizationInfo.setLineWidth(1.0f);
		rasterizationInfo.setCullMode(vk::CullModeFlagBits::eBack);
		rasterizationInfo.setFrontFace(vk::FrontFace::eClockwise);
		rasterizationInfo.setDepthBiasEnable(VK_FALSE);

		depthInfo.setDepthBoundsTestEnable(VK_FALSE);
		depthInfo.setDepthCompareOp(vk::CompareOp::eLessOrEqual);
		depthInfo.setDepthTestEnable(VK_TRUE);
		depthInfo.setDepthWriteEnable(VK_TRUE);
		depthInfo.setMaxDepthBounds(1.0f);
		depthInfo.setMinDepthBounds(0.0f);
		depthInfo.setStencilTestEnable(VK_FALSE);
	}

	void PassInitContext::SetAttachments(uint32_t attachmentIndex, const std::vector<Texture2DPtr>& attachmentArray)
	{
		m_attachments[attachmentIndex] = attachmentArray;
	}

	void PassInitContext::SetDepthAttachments(const std::vector<Texture2DPtr>& depthAttachmentArray)
	{
		m_depthAttachments = depthAttachmentArray;
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
		if (m_owner->m_framebuffers.empty())
		{
			return {};
		}

		if (frameIndex == UINT32_MAX)
		{
			frameIndex = static_cast<uint32_t>( Engine::Get()->GetFrameCount() % m_owner->m_framebuffers.size() );
		}

		return m_owner->m_framebuffers[frameIndex];
	}

	const std::vector<Texture2DPtr>& PassExecuteContext::GetFrameAttachments(uint32_t frameIndex /*= UINT32_MAX*/)
	{
		if (m_attachments.empty())
		{
			return m_attachments[0];
		}

		uint32_t maxAttachmentIndex = 0;
		for (auto& pair : m_attachments)
		{
			if (maxAttachmentIndex < pair.first)
			{
				maxAttachmentIndex = pair.first;
			}
		}

		std::vector<Texture2DPtr> frameAttachments(maxAttachmentIndex + 1);

		for (auto& pair : m_attachments)
		{
			if (pair.second.empty())
			{
				continue;
			}

			uint32_t correctedFrameIndex = frameIndex;
			if (frameIndex == UINT32_MAX)
			{
				correctedFrameIndex = static_cast<uint32_t>(Engine::Get()->GetFrameCount() % pair.second.size());
			}
			frameAttachments[pair.first] = pair.second[correctedFrameIndex];
		}

		return frameAttachments;
	}

	const std::unordered_map<uint32_t, std::vector<Texture2DPtr>>& PassExecuteContext::GetAllAttachments()
	{
		return m_attachments;
	}

	Texture2DPtr PassExecuteContext::GetDepthAttachment(uint32_t frameIndex /*= UINT32_MAX*/)
	{
		if (m_depthAttachments.empty())
		{
			return nullptr;
		}

		if (frameIndex == UINT32_MAX)
		{
			frameIndex = static_cast<uint32_t>( Engine::Get()->GetFrameCount() % m_depthAttachments.size() );
		}

		return m_depthAttachments[frameIndex];
	}

	const std::vector<Texture2DPtr>& PassExecuteContext::GetDepthAttachments()
	{
		return m_depthAttachments;
	}

	PipelineData& PassExecuteContext::FindPipeline(MaterialPtr material)
	{
		return m_owner->CreateOrFindPipeline(*m_owner->m_initContext, material);
	}

}

