#include "GBufferPass.h"
#include "utils/ResourceUtils.h"
#include "data/MeshData.h"
#include "scene/mesh/MeshComponent.h"
#include "DepthPrepass.h"

namespace CGE
{
	using namespace VULKAN_HPP_NAMESPACE;

	GBufferPass::GBufferPass(HashString inName)
		:RenderPassBase(inName)
	{
	}
	
	void GBufferPass::ExecutePass(vk::CommandBuffer* commandBuffer, PassExecuteContext& executeContext, RenderPassDataTable& dataTable)
	{
		//ImageMemoryBarrier depthTextureBarrier = executeContext.GetDepthAttachment()->GetImage().CreateLayoutBarrier(
		//	ImageLayout::eUndefined,
		//	ImageLayout::eDepthAttachmentOptimal,
		//	AccessFlagBits::eShaderRead,
		//	AccessFlagBits::eDepthStencilAttachmentRead,
		//	ImageAspectFlagBits::eDepth | ImageAspectFlagBits::eStencil,
		//	0, 1, 0, 1);
		//commandBuffer->pipelineBarrier(
		//	PipelineStageFlagBits::eComputeShader,
		//	PipelineStageFlagBits::eAllGraphics,
		//	DependencyFlags(),
		//	0, nullptr, 0, nullptr,
		//	1, &depthTextureBarrier);
			
		Scene* scene = Engine::GetSceneInstance();
			
		RenderPassBeginInfo passBeginInfo;
		passBeginInfo.setRenderPass(executeContext.GetRenderPass());
		passBeginInfo.setFramebuffer(executeContext.GetFramebuffer());
		passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), Extent2D(executeContext.GetWidth(), executeContext.GetHeight())));
		passBeginInfo.setClearValueCount(static_cast<uint32_t>( clearValues.size() ));
		passBeginInfo.setPClearValues(clearValues.data());
			
		DeviceSize offset = 0;
		commandBuffer->beginRenderPass(passBeginInfo, SubpassContents::eInline);
			
		//------------------------------------------------------------------------------------------------------------
		for (HashString& shaderHash : scene->GetShadersList())
		{
			PipelineData& pipelineData = executeContext.FindPipeline(scene->GetShaderToMaterial()[shaderHash][0]);
			
			commandBuffer->bindPipeline(PipelineBindPoint::eGraphics, pipelineData.pipeline);
			commandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});
			
			for (MaterialPtr material : scene->GetShaderToMaterial()[shaderHash])
			{
				HashString materialId = material->GetResourceId();
			
				commandBuffer->bindDescriptorSets(
					PipelineBindPoint::eGraphics,
					pipelineData.pipelineLayout,
					1,
					material->GetDescriptorSets().size()-1,
					material->GetDescriptorSets().data()+1,
					0, nullptr);
			
				for (MeshDataPtr meshData : scene->GetMaterialToMeshData()[material->GetResourceId()])
				{
					HashString meshId = meshData->GetResourceId();
			
					commandBuffer->pushConstants(pipelineData.pipelineLayout, ShaderStageFlagBits::eAll, 0, sizeof(uint32_t), & scene->GetMeshDataToIndex(materialId)[meshId]);
					commandBuffer->bindVertexBuffers(0, 1, &meshData->GetVertexBuffer()->GetNativeBuffer(), &offset);
					commandBuffer->bindIndexBuffer(meshData->GetIndexBuffer()->GetNativeBuffer(), 0, IndexType::eUint32);
					commandBuffer->drawIndexed(meshData->GetIndexCount(), static_cast<uint32_t>(scene->GetMeshDataToTransform(materialId)[meshId].size()), 0, 0, 0);
				}
			}		
		}
		//------------------------------------------------------------------------------------------------------------
		commandBuffer->endRenderPass();
	}

	void GBufferPass::InitPass(RenderPassDataTable& dataTable, PassInitContext& initContext)
	{
		// just init clear values
		clearValues[0].setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));
		clearValues[1].setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));
		clearValues[2].setDepthStencil(ClearDepthStencilValue(1.0f, 0));

		auto depthData = dataTable.GetPassData<DepthPrepassData>();
		initContext.SetDepthAttachments(depthData->depthTextures);

		initContext.depthInfo.setDepthTestEnable(VK_TRUE);
		initContext.depthInfo.setDepthCompareOp(vk::CompareOp::eEqual);
		initContext.depthInfo.setDepthWriteEnable(VK_FALSE);
		initContext.depthInfo.setStencilTestEnable(VK_FALSE);

		std::shared_ptr<GBufferPassData> gbufferData = std::make_shared<GBufferPassData>();
		dataTable.AddPassData<GBufferPassData>(gbufferData);

		gbufferData->albedos = ResourceUtils::CreateColorTextureArray(initContext.GetPassName()+HashString("_albedo"), 2, initContext.GetWidth(), initContext.GetHeight());
		gbufferData->normals = ResourceUtils::CreateColorTextureArray(initContext.GetPassName() + HashString("_normal"), 2, initContext.GetWidth(), initContext.GetHeight());

		initContext.SetAttachments(0, gbufferData->albedos);
		initContext.SetAttachments(1, gbufferData->normals);
	}

	//void GBufferPass::RecordCommands(CommandBuffer* inCommandBuffer)
	//{
	//	ImageMemoryBarrier depthTextureBarrier = GetDepthAttachment().CreateLayoutBarrier(
	//		ImageLayout::eUndefined,
	//		ImageLayout::eDepthAttachmentOptimal,
	//		AccessFlagBits::eShaderRead,
	//		AccessFlagBits::eDepthStencilAttachmentRead,
	//		ImageAspectFlagBits::eDepth | ImageAspectFlagBits::eStencil,
	//		0, 1, 0, 1);
	//	inCommandBuffer->pipelineBarrier(
	//		PipelineStageFlagBits::eComputeShader,
	//		PipelineStageFlagBits::eAllGraphics,
	//		DependencyFlags(),
	//		0, nullptr, 0, nullptr,
	//		1, &depthTextureBarrier);
	//
	//	Scene* scene = Engine::GetSceneInstance();
	//
	//	RenderPassBeginInfo passBeginInfo;
	//	passBeginInfo.setRenderPass(GetRenderPass());
	//	passBeginInfo.setFramebuffer(GetFramebuffer());
	//	passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), Extent2D(GetWidth(), GetHeight())));
	//	passBeginInfo.setClearValueCount(static_cast<uint32_t>( clearValues.size() ));
	//	passBeginInfo.setPClearValues(clearValues.data());
	//
	//	DeviceSize offset = 0;
	//	inCommandBuffer->beginRenderPass(passBeginInfo, SubpassContents::eInline);
	//
	//	//------------------------------------------------------------------------------------------------------------
	//	for (HashString& shaderHash : scene->GetShadersList())
	//	{
	//		PipelineData& pipelineData = FindPipeline(scene->GetShaderToMaterial()[shaderHash][0]);
	//
	//		inCommandBuffer->bindPipeline(PipelineBindPoint::eGraphics, pipelineData.pipeline);
	//		inCommandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});
	//
	//		for (MaterialPtr material : scene->GetShaderToMaterial()[shaderHash])
	//		{
	//			HashString materialId = material->GetResourceId();
	//
	//			inCommandBuffer->bindDescriptorSets(
	//				PipelineBindPoint::eGraphics,
	//				pipelineData.pipelineLayout,
	//				1,
	//				material->GetDescriptorSets().size()-1,
	//				material->GetDescriptorSets().data()+1,
	//				0, nullptr);
	//
	//			for (MeshDataPtr meshData : scene->GetMaterialToMeshData()[material->GetResourceId()])
	//			{
	//				HashString meshId = meshData->GetResourceId();
	//
	//				inCommandBuffer->pushConstants(pipelineData.pipelineLayout, ShaderStageFlagBits::eAll, 0, sizeof(uint32_t), & scene->GetMeshDataToIndex(materialId)[meshId]);
	//				inCommandBuffer->bindVertexBuffers(0, 1, &meshData->GetVertexBuffer()->GetNativeBuffer(), &offset);
	//				inCommandBuffer->bindIndexBuffer(meshData->GetIndexBuffer()->GetNativeBuffer(), 0, IndexType::eUint32);
	//				inCommandBuffer->drawIndexed(meshData->GetIndexCount(), static_cast<uint32_t>(scene->GetMeshDataToTransform(materialId)[meshId].size()), 0, 0, 0);
	//			}
	//		}		
	//	}
	//	//------------------------------------------------------------------------------------------------------------
	//	inCommandBuffer->endRenderPass();
	//}
	//
	//void GBufferPass::OnCreate()
	//{
	//	// just init clear values
	//	clearValues[0].setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));
	//	clearValues[1].setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));
	//	clearValues[2].setDepthStencil(ClearDepthStencilValue(1.0f, 0));
	//}
	//
	//RenderPass GBufferPass::CreateRenderPass()
	//{
	//	// albedo
	//	AttachmentDescription albedoAttachDesc;
	//	albedoAttachDesc.setFormat(Format::eR8G8B8A8Unorm);
	//	albedoAttachDesc.setSamples(SampleCountFlagBits::e1);
	//	albedoAttachDesc.setLoadOp(AttachmentLoadOp::eClear);
	//	albedoAttachDesc.setStoreOp(AttachmentStoreOp::eStore);
	//	albedoAttachDesc.setStencilLoadOp(AttachmentLoadOp::eDontCare);
	//	albedoAttachDesc.setStencilStoreOp(AttachmentStoreOp::eDontCare);
	//	albedoAttachDesc.setInitialLayout(ImageLayout::eUndefined);
	//	albedoAttachDesc.setFinalLayout(ImageLayout::eColorAttachmentOptimal);
	//	AttachmentReference albedoAttachRef;
	//	albedoAttachRef.setAttachment(0);
	//	albedoAttachRef.setLayout(ImageLayout::eColorAttachmentOptimal);
	//	// normal
	//	AttachmentDescription normalAttachDesc;
	//	normalAttachDesc.setFormat(Format::eR16G16B16A16Sfloat);
	//	normalAttachDesc.setSamples(SampleCountFlagBits::e1);
	//	normalAttachDesc.setLoadOp(AttachmentLoadOp::eClear);
	//	normalAttachDesc.setStoreOp(AttachmentStoreOp::eStore);
	//	normalAttachDesc.setStencilLoadOp(AttachmentLoadOp::eDontCare);
	//	normalAttachDesc.setStencilStoreOp(AttachmentStoreOp::eDontCare);
	//	normalAttachDesc.setInitialLayout(ImageLayout::eUndefined);
	//	normalAttachDesc.setFinalLayout(ImageLayout::eColorAttachmentOptimal);
	//	AttachmentReference normalAttachRef;
	//	normalAttachRef.setAttachment(1);
	//	normalAttachRef.setLayout(ImageLayout::eColorAttachmentOptimal);
	//
	//	// depth
	//	AttachmentDescription depthAttachDesc;
	//	depthAttachDesc.setFormat(Format::eD24UnormS8Uint);
	//	depthAttachDesc.setSamples(SampleCountFlagBits::e1);
	//	depthAttachDesc.setLoadOp(AttachmentLoadOp::eLoad);
	//	depthAttachDesc.setStoreOp(AttachmentStoreOp::eStore);
	//	depthAttachDesc.setStencilLoadOp(AttachmentLoadOp::eLoad);
	//	depthAttachDesc.setStencilStoreOp(AttachmentStoreOp::eStore);
	//	depthAttachDesc.setInitialLayout(ImageLayout::eUndefined);
	//	depthAttachDesc.setFinalLayout(ImageLayout::eDepthStencilAttachmentOptimal);
	//	AttachmentReference depthAttachRef;
	//	depthAttachRef.setAttachment(2);
	//	depthAttachRef.setLayout(ImageLayout::eDepthStencilAttachmentOptimal);
	//
	//	std::array<AttachmentReference, 2> colorAttachments{ albedoAttachRef, normalAttachRef };
	//	SubpassDescription subpassDesc;
	//	subpassDesc.setColorAttachmentCount(static_cast<uint32_t>(colorAttachments.size()));
	//	subpassDesc.setPColorAttachments(colorAttachments.data());
	//	subpassDesc.setPDepthStencilAttachment(&depthAttachRef);
	//	subpassDesc.setPipelineBindPoint(PipelineBindPoint::eGraphics);
	//
	//	SubpassDependency subpassDependency;
	//	subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
	//	subpassDependency.setDstSubpass(0);
	//	subpassDependency.setSrcStageMask(PipelineStageFlagBits::eColorAttachmentOutput);
	//	subpassDependency.setSrcAccessMask(AccessFlags());
	//	subpassDependency.setDstStageMask(PipelineStageFlagBits::eColorAttachmentOutput);
	//	subpassDependency.setDstAccessMask(AccessFlagBits::eColorAttachmentRead | AccessFlagBits::eColorAttachmentWrite);
	//
	//	std::array<AttachmentDescription, 3> attachDescriptions = { albedoAttachDesc, normalAttachDesc, depthAttachDesc };
	//	RenderPassCreateInfo renderPassInfo;
	//	renderPassInfo.setAttachmentCount(static_cast<uint32_t>(attachDescriptions.size()));
	//	renderPassInfo.setPAttachments(attachDescriptions.data());
	//	renderPassInfo.setSubpassCount(1);
	//	renderPassInfo.setPSubpasses(&subpassDesc);
	//	renderPassInfo.setDependencyCount(1);
	//	renderPassInfo.setPDependencies(&subpassDependency);
	//
	//	return GetVulkanDevice()->GetDevice().createRenderPass(renderPassInfo);
	//}
	//
	//void GBufferPass::CreateColorAttachments(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight)
	//{
	//	VulkanDevice* vulkanDevice = GetVulkanDevice();
	//
	//	// albedo
	//	VulkanImage albedoAttachmentImage = ResourceUtils::CreateColorAttachment(vulkanDevice, inWidth, inHeight);
	//	outAttachments.push_back(albedoAttachmentImage);
	//	outAttachmentViews.push_back( albedoAttachmentImage.CreateView({ ImageAspectFlagBits::eColor, 0, 1, 0, 1 }, ImageViewType::e2D) );
	//	// normal
	//	VulkanImage normalAttachmentImage = ResourceUtils::CreateColorAttachment(vulkanDevice, inWidth, inHeight, vk::Format::eR16G16B16A16Sfloat);
	//	outAttachments.push_back(normalAttachmentImage);
	//	outAttachmentViews.push_back(normalAttachmentImage.CreateView({ ImageAspectFlagBits::eColor, 0, 1, 0, 1 }, ImageViewType::e2D));
	//}
	//
	//void GBufferPass::CreateDepthAttachment(VulkanImage& outDepthAttachment, ImageView& outDepthAttachmentView, uint32_t inWidth, uint32_t inHeight)
	//{
	//	outDepthAttachment = ResourceUtils::CreateDepthAttachment(GetVulkanDevice(), inWidth, inHeight);
	//	outDepthAttachmentView = outDepthAttachment.CreateView({ ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }, ImageViewType::e2D);
	//}
	//
	//Pipeline GBufferPass::CreatePipeline(MaterialPtr inMaterial, PipelineLayout inLayout, RenderPass inRenderPass)
	//{
	//	std::vector<PipelineShaderStageCreateInfo> shaderStageInfoArray = { inMaterial->GetVertexStageInfo(), inMaterial->GetFragmentStageInfo() };
	//
	//	VertexInputBindingDescription bindingDesc = MeshData::GetBindingDescription(0);
	//	std::array<VertexInputAttributeDescription, 5> attributeDesc = Vertex::GetAttributeDescriptions(0);
	//	PipelineVertexInputStateCreateInfo vertexInputInfo;
	//	vertexInputInfo.setVertexBindingDescriptionCount(1);
	//	vertexInputInfo.setPVertexBindingDescriptions(&bindingDesc);
	//	vertexInputInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDesc.size()));
	//	vertexInputInfo.setPVertexAttributeDescriptions(attributeDesc.data());
	//
	//	PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	//	inputAssemblyInfo.setTopology(PrimitiveTopology::eTriangleList);
	//	inputAssemblyInfo.setPrimitiveRestartEnable(VK_FALSE);
	//
	//	Viewport viewport;
	//	viewport.setX(0.0f);
	//	viewport.setY(0.0f);
	//	viewport.setWidth(static_cast<float>(GetWidth()));
	//	viewport.setHeight(static_cast<float>(GetHeight()));
	//	viewport.setMinDepth(0.0f);
	//	viewport.setMaxDepth(1.0f);
	//
	//	Rect2D scissor;
	//	scissor.setOffset(Offset2D(0, 0));
	//	scissor.setExtent(Extent2D{ GetWidth(), GetHeight() });
	//
	//	PipelineViewportStateCreateInfo viewportInfo;
	//	viewportInfo.setViewportCount(1);
	//	viewportInfo.setPViewports(&viewport);
	//	viewportInfo.setScissorCount(1);
	//	viewportInfo.setPScissors(&scissor);
	//
	//	PipelineRasterizationStateCreateInfo rasterizationInfo;
	//	rasterizationInfo.setDepthClampEnable(VK_FALSE);
	//	rasterizationInfo.setRasterizerDiscardEnable(VK_FALSE);
	//	rasterizationInfo.setPolygonMode(PolygonMode::eFill);
	//	rasterizationInfo.setLineWidth(1.0f);
	//	rasterizationInfo.setCullMode(CullModeFlagBits::eBack);
	//	rasterizationInfo.setFrontFace(FrontFace::eClockwise);
	//	rasterizationInfo.setDepthBiasEnable(VK_FALSE);
	//
	//	PipelineMultisampleStateCreateInfo multisampleInfo;
	//
	//	PipelineDepthStencilStateCreateInfo depthStencilInfo;
	//	depthStencilInfo.setDepthBoundsTestEnable(VK_FALSE);
	//	depthStencilInfo.setDepthCompareOp(CompareOp::eEqual);
	//	depthStencilInfo.setDepthTestEnable(VK_TRUE);
	//	depthStencilInfo.setDepthWriteEnable(VK_FALSE);
	//	//depthStencilInfo.setMaxDepthBounds(1.0f);
	//	//depthStencilInfo.setMinDepthBounds(0.0f);
	//	depthStencilInfo.setStencilTestEnable(VK_FALSE);
	//
	//	PipelineColorBlendAttachmentState albedoBlendAttachment;
	//	albedoBlendAttachment.setColorWriteMask(ColorComponentFlagBits::eR | ColorComponentFlagBits::eG | ColorComponentFlagBits::eB | ColorComponentFlagBits::eA);
	//	albedoBlendAttachment.setBlendEnable(VK_FALSE);
	//	albedoBlendAttachment.setSrcColorBlendFactor(BlendFactor::eOne);
	//	albedoBlendAttachment.setDstColorBlendFactor(BlendFactor::eZero);
	//	albedoBlendAttachment.setColorBlendOp(BlendOp::eAdd);
	//	albedoBlendAttachment.setSrcAlphaBlendFactor(BlendFactor::eOne);
	//	albedoBlendAttachment.setDstAlphaBlendFactor(BlendFactor::eZero);
	//	albedoBlendAttachment.setAlphaBlendOp(BlendOp::eAdd);
	//	PipelineColorBlendAttachmentState normalBlendAttachment;
	//	normalBlendAttachment.setColorWriteMask(ColorComponentFlagBits::eR | ColorComponentFlagBits::eG | ColorComponentFlagBits::eB | ColorComponentFlagBits::eA);
	//	normalBlendAttachment.setBlendEnable(VK_FALSE);
	//	normalBlendAttachment.setSrcColorBlendFactor(BlendFactor::eOne);
	//	normalBlendAttachment.setDstColorBlendFactor(BlendFactor::eZero);
	//	normalBlendAttachment.setColorBlendOp(BlendOp::eAdd);
	//	normalBlendAttachment.setSrcAlphaBlendFactor(BlendFactor::eOne);
	//	normalBlendAttachment.setDstAlphaBlendFactor(BlendFactor::eZero);
	//	normalBlendAttachment.setAlphaBlendOp(BlendOp::eAdd);
	//
	//	std::array<PipelineColorBlendAttachmentState, 2> colorBlendAttachmentStates{ albedoBlendAttachment, normalBlendAttachment };
	//	PipelineColorBlendStateCreateInfo colorBlendInfo;
	//	colorBlendInfo.setLogicOpEnable(VK_FALSE);
	//	colorBlendInfo.setLogicOp(LogicOp::eCopy);
	//	colorBlendInfo.setAttachmentCount(static_cast<uint32_t>( colorBlendAttachmentStates.size() ));
	//	colorBlendInfo.setPAttachments(colorBlendAttachmentStates.data());
	//	colorBlendInfo.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });
	//
	//	GraphicsPipelineCreateInfo pipelineInfo;
	//	pipelineInfo.setStageCount(2);
	//	pipelineInfo.setPStages(shaderStageInfoArray.data());
	//	pipelineInfo.setPVertexInputState(&vertexInputInfo);
	//	pipelineInfo.setPInputAssemblyState(&inputAssemblyInfo);
	//	pipelineInfo.setPViewportState(&viewportInfo);
	//	pipelineInfo.setPRasterizationState(&rasterizationInfo);
	//	pipelineInfo.setPMultisampleState(&multisampleInfo);
	//	pipelineInfo.setPDepthStencilState(&depthStencilInfo);
	//	pipelineInfo.setPColorBlendState(&colorBlendInfo);
	//	pipelineInfo.setPDynamicState(nullptr);
	//	pipelineInfo.setLayout(inLayout);
	//	pipelineInfo.setRenderPass(inRenderPass);
	//	pipelineInfo.setSubpass(0);
	//	pipelineInfo.setBasePipelineHandle(Pipeline());
	//	pipelineInfo.setBasePipelineIndex(-1);
	//
	//	return GetVulkanDevice()->GetDevice().createGraphicsPipeline(GetVulkanDevice()->GetPipelineCache(), pipelineInfo).value;
	//}
	
}
