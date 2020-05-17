#include "PostProcessPass.h"
#include "../Renderer.h"
#include "data/DataManager.h"
#include "GBufferPass.h"
#include "../DataStructures.h"

PostProcessPass::PostProcessPass(HashString inName)
	: VulkanPassBase(inName)
{

}

void PostProcessPass::Draw(CommandBuffer* inCommandBuffer)
{
	VulkanSwapChain& swapChain = GetRenderer()->GetSwapChain();
	MeshDataPtr meshData = MeshData::FullscreenQuad();

	PipelineData& pipelineData = FindGraphicsPipeline(postProcessMaterial);

	ClearValue clearValue;
	clearValue.setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));

	RenderPassBeginInfo passBeginInfo;
	passBeginInfo.setRenderPass(swapChain.GetRenderPass());
	passBeginInfo.setFramebuffer(swapChain.GetFramebuffer());
	passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), GetRenderer()->GetSwapChain().GetExtent()));
	passBeginInfo.setClearValueCount(1);
	passBeginInfo.setPClearValues(&clearValue);

	DeviceSize offset = 0;
	inCommandBuffer->beginRenderPass(passBeginInfo, SubpassContents::eInline);
	inCommandBuffer->bindPipeline(PipelineBindPoint::eGraphics, pipelineData.pipeline);
	inCommandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});

	inCommandBuffer->bindVertexBuffers(0, 1, &meshData->GetVertexBuffer().GetBuffer(), &offset);
	inCommandBuffer->bindIndexBuffer(meshData->GetIndexBuffer().GetBuffer(), 0, IndexType::eUint32);
	inCommandBuffer->drawIndexed(meshData->GetIndexCount(), 1, 0, 0, 0);
	inCommandBuffer->endRenderPass();
}

void PostProcessPass::OnCreate()
{
	GBufferPass* gBufferPass = GetRenderer()->GetGBufferPass();
	screenImage = ObjectBase::NewObject<Texture2D, const HashString&>("SceneImageTexture");
	screenImage->CreateFromExternal(gBufferPass->GetAttachments()[0], gBufferPass->GetAttachmentViews()[0]);

	postProcessMaterial = DataManager::RequestResourceType<Material>(
		"PostProcessMaterial",
		"content/shaders/PostProcessVert.spv",
		"content/shaders/PostProcessFrag.spv"
		);
	ObjectMVPData objData;
	postProcessMaterial->SetUniformBuffer<ObjectMVPData>("mvpBuffer", objData);
	postProcessMaterial->SetTexture("screenImage", screenImage);
	postProcessMaterial->LoadResources();
}

RenderPass PostProcessPass::CreateRenderPass()
{
	SubpassDescription subpassDesc;
	subpassDesc.setColorAttachmentCount(0);
	subpassDesc.setPColorAttachments(nullptr);
	subpassDesc.setPDepthStencilAttachment(nullptr);
	subpassDesc.setPipelineBindPoint(PipelineBindPoint::eGraphics);

	SubpassDependency subpassDependency;
	subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
	subpassDependency.setDstSubpass(0);
	subpassDependency.setSrcStageMask(PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependency.setSrcAccessMask(AccessFlags());
	subpassDependency.setDstStageMask(PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependency.setDstAccessMask(AccessFlagBits::eColorAttachmentRead | AccessFlagBits::eColorAttachmentWrite);

	RenderPassCreateInfo renderPassInfo;
	renderPassInfo.setAttachmentCount(0);
	renderPassInfo.setPAttachments(nullptr);
	renderPassInfo.setSubpassCount(1);
	renderPassInfo.setPSubpasses(&subpassDesc);
	renderPassInfo.setDependencyCount(1);
	renderPassInfo.setPDependencies(&subpassDependency);

	return GetVulkanDevice()->GetDevice().createRenderPass(renderPassInfo);
}

void PostProcessPass::CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight)
{
}

void PostProcessPass::CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight)
{
}

Pipeline PostProcessPass::CreateGraphicsPipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass)
{
	VulkanSwapChain& swapChain = GetRenderer()->GetSwapChain();

	PipelineShaderStageCreateInfo vertStageInfo;
	vertStageInfo.setStage(ShaderStageFlagBits::eVertex);
	vertStageInfo.setModule(inMaterial->GetVertexShader()->GetShaderModule());
	vertStageInfo.setPName("main");

	PipelineShaderStageCreateInfo fragStageInfo;
	fragStageInfo.setStage(ShaderStageFlagBits::eFragment);
	fragStageInfo.setModule(inMaterial->GetFragmentShader()->GetShaderModule());
	fragStageInfo.setPName("main");

	std::vector<PipelineShaderStageCreateInfo> shaderStageInfoArray = { vertStageInfo, fragStageInfo };

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
	viewport.setWidth((float)swapChain.GetExtent().width);
	viewport.setHeight((float)swapChain.GetExtent().height);
	viewport.setMinDepth(0.0f);
	viewport.setMaxDepth(1.0f);

	Rect2D scissor;
	scissor.setOffset(Offset2D(0, 0));
	scissor.setExtent(swapChain.GetExtent());

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
	rasterizationInfo.setCullMode(CullModeFlagBits::eNone);
	rasterizationInfo.setFrontFace(FrontFace::eClockwise);
	rasterizationInfo.setDepthBiasEnable(VK_FALSE);

	PipelineMultisampleStateCreateInfo multisampleInfo;

	PipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.setColorWriteMask(ColorComponentFlagBits::eR | ColorComponentFlagBits::eG | ColorComponentFlagBits::eB | ColorComponentFlagBits::eA);
	colorBlendAttachment.setBlendEnable(VK_FALSE);
	colorBlendAttachment.setSrcColorBlendFactor(BlendFactor::eOne);
	colorBlendAttachment.setDstColorBlendFactor(BlendFactor::eZero);
	colorBlendAttachment.setColorBlendOp(BlendOp::eAdd);
	colorBlendAttachment.setSrcAlphaBlendFactor(BlendFactor::eOne);
	colorBlendAttachment.setDstAlphaBlendFactor(BlendFactor::eZero);
	colorBlendAttachment.setAlphaBlendOp(BlendOp::eAdd);

	PipelineColorBlendStateCreateInfo colorBlendInfo;
	colorBlendInfo.setLogicOpEnable(VK_FALSE);
	colorBlendInfo.setLogicOp(LogicOp::eCopy);
	colorBlendInfo.setAttachmentCount(1);
	colorBlendInfo.setPAttachments(&colorBlendAttachment);
	colorBlendInfo.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

	GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.setStageCount(2);
	pipelineInfo.setPStages(shaderStageInfoArray.data());
	pipelineInfo.setPVertexInputState(&vertexInputInfo);
	pipelineInfo.setPInputAssemblyState(&inputAssemblyInfo);
	pipelineInfo.setPViewportState(&viewportInfo);
	pipelineInfo.setPRasterizationState(&rasterizationInfo);
	pipelineInfo.setPMultisampleState(&multisampleInfo);
	pipelineInfo.setPDepthStencilState(nullptr);
	pipelineInfo.setPColorBlendState(&colorBlendInfo);
	pipelineInfo.setPDynamicState(nullptr);
	pipelineInfo.setLayout(inLayout);
	pipelineInfo.setRenderPass(GetRenderer()->GetSwapChain().GetRenderPass());
	pipelineInfo.setSubpass(0);
	pipelineInfo.setBasePipelineHandle(Pipeline());
	pipelineInfo.setBasePipelineIndex(-1);

	return GetVulkanDevice()->GetDevice().createGraphicsPipeline(GetVulkanDevice()->GetPipelineCache(), pipelineInfo);
}
