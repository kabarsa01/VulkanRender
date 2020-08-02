#include "ZPrepass.h"
#include "utils/ImageUtils.h"
#include "data/MeshData.h"
#include "scene/mesh/MeshComponent.h"

ZPrepass::ZPrepass(HashString inName)
	: VulkanPassBase(inName)
{

}

void ZPrepass::RecordCommands(CommandBuffer* inCommandBuffer)
{
	// barriers ----------------------------------------------
	ImageMemoryBarrier depthTextureBarrier = GetDepthAttachment().CreateLayoutBarrier(
		ImageLayout::eUndefined,
		ImageLayout::eDepthStencilAttachmentOptimal,
		AccessFlagBits::eShaderRead,
		AccessFlagBits::eShaderWrite,
		ImageAspectFlagBits::eDepth | ImageAspectFlagBits::eStencil,
		0, 1, 0, 1);
	std::array<ImageMemoryBarrier, 1> barriers{ depthTextureBarrier };
	inCommandBuffer->pipelineBarrier(
		PipelineStageFlagBits::eFragmentShader,
		PipelineStageFlagBits::eVertexShader,
		DependencyFlags(),
		0, nullptr, 0, nullptr,
		static_cast<uint32_t>(barriers.size()), barriers.data());

	ScenePtr scene = Engine::GetSceneInstance();

	RenderPassBeginInfo passBeginInfo;
	passBeginInfo.setRenderPass(GetRenderPass());
	passBeginInfo.setFramebuffer(GetFramebuffer());
	passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), Extent2D(GetWidth(), GetHeight())));
	passBeginInfo.setClearValueCount(1);
	passBeginInfo.setPClearValues(&clearValue);

	DeviceSize offset = 0;
	inCommandBuffer->beginRenderPass(passBeginInfo, SubpassContents::eInline);

	//------------------------------------------------------------------------------------------------------------
	for (HashString& shaderHash : scene->GetShadersList())
	{
		PipelineData& pipelineData = FindPipeline(scene->GetShaderToMaterial()[shaderHash][0]);

		inCommandBuffer->bindPipeline(PipelineBindPoint::eGraphics, pipelineData.pipeline);
		inCommandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});

		for (MaterialPtr material : scene->GetShaderToMaterial()[shaderHash])
		{
			HashString materialId = material->GetResourceId();

			material->CreateDescriptorSet(GetVulkanDevice());
			inCommandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 1, material->GetDescriptorSets(), {});

			for (MeshDataPtr meshData : scene->GetMaterialToMeshData()[material->GetResourceId()])
			{
				HashString meshId = meshData->GetResourceId();

				inCommandBuffer->pushConstants(pipelineData.pipelineLayout, ShaderStageFlagBits::eAllGraphics, 0, sizeof(uint32_t), &scene->GetMeshDataToIndex(materialId)[meshId]);
				inCommandBuffer->bindVertexBuffers(0, 1, &meshData->GetVertexBuffer().GetBuffer(), &offset);
				inCommandBuffer->bindIndexBuffer(meshData->GetIndexBuffer().GetBuffer(), 0, IndexType::eUint32);
				inCommandBuffer->drawIndexed(meshData->GetIndexCount(), scene->GetMeshDataToTransform(materialId)[meshId].size(), 0, 0, 0);
			}
		}
	}
	//------------------------------------------------------------------------------------------------------------
	inCommandBuffer->endRenderPass();
}

void ZPrepass::OnCreate()
{
	clearValue.setDepthStencil(ClearDepthStencilValue(1.0f, 0));
}

RenderPass ZPrepass::CreateRenderPass()
{
	// depth
	AttachmentDescription depthAttachDesc;
	depthAttachDesc.setFormat(Format::eD24UnormS8Uint);
	depthAttachDesc.setSamples(SampleCountFlagBits::e1);
	depthAttachDesc.setLoadOp(AttachmentLoadOp::eClear);
	depthAttachDesc.setStoreOp(AttachmentStoreOp::eStore);
	depthAttachDesc.setStencilLoadOp(AttachmentLoadOp::eClear);
	depthAttachDesc.setStencilStoreOp(AttachmentStoreOp::eStore);
	depthAttachDesc.setInitialLayout(ImageLayout::eUndefined);
	depthAttachDesc.setFinalLayout(ImageLayout::eDepthStencilAttachmentOptimal);
	AttachmentReference depthAttachRef;
	depthAttachRef.setAttachment(0);
	depthAttachRef.setLayout(ImageLayout::eDepthStencilAttachmentOptimal);

	SubpassDescription subpassDesc;
	subpassDesc.setColorAttachmentCount(0);
	subpassDesc.setPColorAttachments(nullptr);
	subpassDesc.setPDepthStencilAttachment(&depthAttachRef);
	subpassDesc.setPipelineBindPoint(PipelineBindPoint::eGraphics);

	SubpassDependency subpassDependency;
	subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
	subpassDependency.setDstSubpass(0);
	subpassDependency.setSrcStageMask(PipelineStageFlagBits::eAllGraphics);
	subpassDependency.setDstStageMask(PipelineStageFlagBits::eAllGraphics);
	subpassDependency.setSrcAccessMask(AccessFlagBits::eDepthStencilAttachmentWrite);
	subpassDependency.setDstAccessMask(AccessFlagBits::eMemoryRead);

	RenderPassCreateInfo renderPassInfo;
	renderPassInfo.setAttachmentCount(1);
	renderPassInfo.setPAttachments(&depthAttachDesc);
	renderPassInfo.setSubpassCount(1);
	renderPassInfo.setPSubpasses(&subpassDesc);
	renderPassInfo.setDependencyCount(1);
	renderPassInfo.setPDependencies(&subpassDependency);

	return GetVulkanDevice()->GetDevice().createRenderPass(renderPassInfo);
}

void ZPrepass::CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight)
{
}

void ZPrepass::CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight)
{
	outDepthAttachment = ImageUtils::CreateDepthAttachment(GetVulkanDevice(), inWidth, inHeight);
	outDepthAttachmentView = outDepthAttachment.CreateView({ ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }, ImageViewType::e2D);
}

Pipeline ZPrepass::CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass)
{
	std::vector<PipelineShaderStageCreateInfo> shaderStageInfoArray = { inMaterial->GetVertexStageInfo() };

	VertexInputBindingDescription bindingDesc = MeshData::GetBindingDescription(0);
	std::array<VertexInputAttributeDescription, 5> attributeDesc = Vertex::GetAttributeDescriptions(0);
	PipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.setVertexBindingDescriptionCount(1);
	vertexInputInfo.setPVertexBindingDescriptions(&bindingDesc);
	vertexInputInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDesc.size()));
	vertexInputInfo.setPVertexAttributeDescriptions(attributeDesc.data());

	PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	inputAssemblyInfo.setTopology(PrimitiveTopology::eTriangleList);
	inputAssemblyInfo.setPrimitiveRestartEnable(VK_FALSE);

	Viewport viewport;
	viewport.setX(0.0f);
	viewport.setY(0.0f);
	viewport.setWidth(static_cast<float>(GetWidth()));
	viewport.setHeight(static_cast<float>(GetHeight()));
	viewport.setMinDepth(0.0f);
	viewport.setMaxDepth(1.0f);

	Rect2D scissor;
	scissor.setOffset(Offset2D(0, 0));
	scissor.setExtent(Extent2D{ GetWidth(), GetHeight() });

	PipelineViewportStateCreateInfo viewportInfo;
	viewportInfo.setViewportCount(1);
	viewportInfo.setPViewports(&viewport);
	viewportInfo.setScissorCount(1);
	viewportInfo.setPScissors(&scissor);

	PipelineRasterizationStateCreateInfo rasterizationInfo;
	rasterizationInfo.setDepthClampEnable(VK_FALSE);
	rasterizationInfo.setRasterizerDiscardEnable(VK_FALSE);
	rasterizationInfo.setPolygonMode(PolygonMode::eFill);
	rasterizationInfo.setLineWidth(1.0f);
	rasterizationInfo.setCullMode(CullModeFlagBits::eBack);
	rasterizationInfo.setFrontFace(FrontFace::eClockwise);
	rasterizationInfo.setDepthBiasEnable(VK_FALSE);

	PipelineMultisampleStateCreateInfo multisampleInfo;

	PipelineDepthStencilStateCreateInfo depthStencilInfo;
	depthStencilInfo.setDepthBoundsTestEnable(VK_FALSE);
	depthStencilInfo.setDepthCompareOp(CompareOp::eLessOrEqual);
	depthStencilInfo.setDepthTestEnable(VK_TRUE);
	depthStencilInfo.setDepthWriteEnable(VK_TRUE);
	//depthStencilInfo.setMaxDepthBounds(1.0f);
	//depthStencilInfo.setMinDepthBounds(0.0f);
	depthStencilInfo.setStencilTestEnable(VK_FALSE);

	GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.setStageCount(static_cast<uint32_t>( shaderStageInfoArray.size() ));
	pipelineInfo.setPStages(shaderStageInfoArray.data());
	pipelineInfo.setPVertexInputState(&vertexInputInfo);
	pipelineInfo.setPInputAssemblyState(&inputAssemblyInfo);
	pipelineInfo.setPViewportState(&viewportInfo);
	pipelineInfo.setPRasterizationState(&rasterizationInfo);
	pipelineInfo.setPMultisampleState(&multisampleInfo);
	pipelineInfo.setPDepthStencilState(&depthStencilInfo);
	pipelineInfo.setPColorBlendState(nullptr);
	pipelineInfo.setPDynamicState(nullptr);
	pipelineInfo.setLayout(inLayout);
	pipelineInfo.setRenderPass(inRenderPass);
	pipelineInfo.setSubpass(0);
	pipelineInfo.setBasePipelineHandle(Pipeline());
	pipelineInfo.setBasePipelineIndex(-1);

	return GetVulkanDevice()->GetDevice().createGraphicsPipeline(GetVulkanDevice()->GetPipelineCache(), pipelineInfo);
}

