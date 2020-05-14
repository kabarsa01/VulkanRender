#include "VulkanPassBase.h"
#include "../Renderer.h"
#include "core/Engine.h"
#include "../shader/Shader.h"
#include "../shader/VulkanShaderModule.h"
#include "scene/camera/CameraComponent.h"
#include "scene/mesh/MeshComponent.h"
#include "scene/SceneObjectComponent.h"
#include "scene/SceneObjectBase.h"
#include "../DataStructures.h"
#include "scene/Transform.h"
#include "data/DataManager.h"
#include "../PerFrameData.h"

VulkanPassBase::VulkanPassBase(HashString inName)
	: name(inName)
{

}

VulkanPassBase::~VulkanPassBase()
{

}

void VulkanPassBase::Create()
{
	renderer = Engine::GetRendererInstance();
	vulkanDevice = &renderer->GetVulkanDevice();
	width = renderer->GetWidth();
	height = renderer->GetHeight();

	renderPass = CreateRenderPass();
	CreateFramebufferResources(attachments, attachmentViews, width, height);
	framebuffer = CreateFramebuffer(renderPass, attachmentViews, width, height);
}

void VulkanPassBase::Destroy()
{
	Device& device = vulkanDevice->GetDevice();

	device.destroyRenderPass(renderPass);
	device.destroyFramebuffer(framebuffer);
	for (uint32_t index = 0; index < attachments.size(); index++)
	{
		attachments[index].Destroy();
		device.destroyImageView( attachmentViews[index] );
	}
}

void VulkanPassBase::Draw(CommandBuffer* inCommandBuffer)
{
	ScenePtr scene = Engine::GetSceneInstance();
	std::vector<MeshComponentPtr> meshComponents = scene->GetSceneComponentsCast<MeshComponent>();
	//---------------------------------------------------------------------------------
	// TODO: provide some more robust centralized solution for batching/sorting
	std::map<HashString, std::vector<MaterialPtr>> shaderSortedMaterials;
	std::map<HashString, std::vector<MeshDataPtr>> materialSortedMeshes;
	for (MeshComponentPtr meshComp : meshComponents)
	{
		MaterialPtr material = meshComp->material;
		shaderSortedMaterials[material->GetShaderHash()].push_back(material);
		materialSortedMeshes[material->GetResourceId()].push_back(meshComp->meshData);
	}
	//---------------------------------------------------------------------------------

	ClearValue clearValue;
	clearValue.setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));

	RenderPassBeginInfo passBeginInfo;
	passBeginInfo.setRenderPass(renderPass);
	passBeginInfo.setFramebuffer(framebuffer);
	passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), Extent2D(width, height)));
	passBeginInfo.setClearValueCount(1);
	passBeginInfo.setPClearValues(&clearValue);

	DeviceSize offset = 0;
	inCommandBuffer->beginRenderPass(passBeginInfo, SubpassContents::eInline);

	//------------------------------------------------------------------------------------------------------------
	for (auto& shaderMaterialPair : shaderSortedMaterials)
	{
		PipelineData& pipelineData = FindGraphicsPipeline(shaderMaterialPair.second[0]);

		inCommandBuffer->bindPipeline(PipelineBindPoint::eGraphics, pipelineData.pipeline);
		inCommandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, pipelineData.descriptorSets, {});

		for (MaterialPtr material : shaderMaterialPair.second)
		{
			material->CreateDescriptorSet(vulkanDevice);
			inCommandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 1, material->GetDescriptorSets(), {});

			std::vector<MeshDataPtr>& meshes = materialSortedMeshes[material->GetResourceId()];
			for (uint64_t index = 0; index < meshes.size(); index++)
			{
				MeshDataPtr meshData = meshes[index];
				inCommandBuffer->bindVertexBuffers(0, 1, &meshData->GetVertexBuffer().GetBuffer(), &offset);
				inCommandBuffer->bindIndexBuffer(meshData->GetIndexBuffer().GetBuffer(), 0, IndexType::eUint32);
				inCommandBuffer->drawIndexed(meshData->GetIndexCount(), 1, 0, 0, 0);
			}
		}
	}
	//------------------------------------------------------------------------------------------------------------
	inCommandBuffer->endRenderPass();
}

RenderPass VulkanPassBase::CreateRenderPass()
{
	Device& device = vulkanDevice->GetDevice();

	AttachmentDescription colorAttachDesc;
	colorAttachDesc.setFormat(Format::eR16G16B16A16Sfloat);
	colorAttachDesc.setSamples(SampleCountFlagBits::e1);
	colorAttachDesc.setLoadOp(AttachmentLoadOp::eClear);
	colorAttachDesc.setStoreOp(AttachmentStoreOp::eStore);
	colorAttachDesc.setStencilLoadOp(AttachmentLoadOp::eDontCare);
	colorAttachDesc.setStencilStoreOp(AttachmentStoreOp::eDontCare);
	colorAttachDesc.setInitialLayout(ImageLayout::eUndefined);
	colorAttachDesc.setFinalLayout(ImageLayout::eColorAttachmentOptimal);

	AttachmentReference colorAttachRef;
	colorAttachRef.setAttachment(0);
	colorAttachRef.setLayout(ImageLayout::eColorAttachmentOptimal);

	SubpassDescription subpassDesc;
	subpassDesc.setColorAttachmentCount(1);
	subpassDesc.setPColorAttachments(&colorAttachRef);
	subpassDesc.setPipelineBindPoint(PipelineBindPoint::eGraphics);

	SubpassDependency subpassDependency;
	subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
	subpassDependency.setDstSubpass(0);
	subpassDependency.setSrcStageMask(PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependency.setSrcAccessMask(AccessFlags());
	subpassDependency.setDstStageMask(PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependency.setDstAccessMask(AccessFlagBits::eColorAttachmentRead | AccessFlagBits::eColorAttachmentWrite);

	RenderPassCreateInfo renderPassInfo;
	renderPassInfo.setAttachmentCount(1);
	renderPassInfo.setPAttachments(&colorAttachDesc);
	renderPassInfo.setSubpassCount(1);
	renderPassInfo.setPSubpasses(&subpassDesc);
	renderPassInfo.setDependencyCount(1);
	renderPassInfo.setPDependencies(&subpassDependency);

	return device.createRenderPass(renderPassInfo);
}

Framebuffer VulkanPassBase::CreateFramebuffer(RenderPass inRenderPass, std::vector<ImageView>& inAttachmentViews, uint32_t inWidth, uint32_t inHeight)
{
	FramebufferCreateInfo framebufferInfo;
	framebufferInfo.setRenderPass(inRenderPass);
	framebufferInfo.setAttachmentCount(static_cast<uint32_t>( inAttachmentViews.size() ));
	framebufferInfo.setPAttachments(inAttachmentViews.data());
	framebufferInfo.setWidth(inWidth);
	framebufferInfo.setHeight(inHeight);
	framebufferInfo.setLayers(1);

	return vulkanDevice->GetDevice().createFramebuffer(framebufferInfo);
}

void VulkanPassBase::CreateFramebufferResources(std::vector<VulkanImage>& outAttachments, std::vector<ImageView>& outAttachmentViews, uint32_t inWidth, uint32_t inHeight)
{
	Device& device = vulkanDevice->GetDevice();

	uint32_t queueFailyIndices[] = { vulkanDevice->GetGraphicsQueueIndex() };

	VulkanImage colorAttachmentImage;
	colorAttachmentImage.createInfo.setArrayLayers(1);
	colorAttachmentImage.createInfo.setFormat(Format::eR16G16B16A16Sfloat);
	colorAttachmentImage.createInfo.setImageType(ImageType::e2D);
	colorAttachmentImage.createInfo.setInitialLayout(ImageLayout::eUndefined);
	colorAttachmentImage.createInfo.setSamples(SampleCountFlagBits::e1);
	colorAttachmentImage.createInfo.setMipLevels(1);
	colorAttachmentImage.createInfo.setSharingMode(SharingMode::eExclusive);
	colorAttachmentImage.createInfo.setQueueFamilyIndexCount(1);
	colorAttachmentImage.createInfo.setPQueueFamilyIndices(queueFailyIndices);
	colorAttachmentImage.createInfo.setTiling(ImageTiling::eOptimal);
	colorAttachmentImage.createInfo.setFlags(ImageCreateFlags());
	colorAttachmentImage.createInfo.setExtent(Extent3D(inWidth, inHeight, 1));
	colorAttachmentImage.createInfo.setUsage(ImageUsageFlagBits::eColorAttachment | ImageUsageFlagBits::eSampled | ImageUsageFlagBits::eInputAttachment);
	colorAttachmentImage.Create(vulkanDevice);
	colorAttachmentImage.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
	// push to list
	outAttachments.push_back(colorAttachmentImage);

	ImageView view = colorAttachmentImage.CreateView({ ImageAspectFlagBits::eColor, 0, 1, 0, 1 }, ImageViewType::e2D);
	// push to list
	outAttachmentViews.push_back( view );
}

PipelineLayout VulkanPassBase::CreatePipelineLayout(std::vector<DescriptorSetLayout>& inDescriptorSetLayouts)
{
	Device& device = vulkanDevice->GetDevice();

	PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setFlags(PipelineLayoutCreateFlags());
	pipelineLayoutInfo.setSetLayoutCount(static_cast<uint32_t>( inDescriptorSetLayouts.size() ));
	pipelineLayoutInfo.setPSetLayouts(inDescriptorSetLayouts.data());

	return device.createPipelineLayout(pipelineLayoutInfo);
}

PipelineData& VulkanPassBase::FindGraphicsPipeline(MaterialPtr inMaterial)
{
	Device& device = vulkanDevice->GetDevice();

	PipelineRegistry& pipelineRegistry = *PipelineRegistry::GetInstance();
	// check pipeline storage and create new pipeline in case it was not created before
	if (!pipelineRegistry.HasPipeline(inMaterial->GetShaderHash(), name))
	{
		PipelineData pipelineData;

		inMaterial->CreateDescriptorSet(vulkanDevice);
		pipelineData.descriptorSets = { renderer->GetPerFrameData()->GetSet(), inMaterial->GetDescriptorSet() };

		std::vector<DescriptorSetLayout> setLayouts = { renderer->GetPerFrameData()->GetLayout(), inMaterial->GetDescriptorSetLayout() };
		pipelineData.pipelineLayout = CreatePipelineLayout(setLayouts);
		pipelineData.pipeline = CreateGraphicsPipeline(inMaterial, pipelineData.pipelineLayout);

		pipelineRegistry.StorePipeline(inMaterial->GetShaderHash(), name, pipelineData);
	}

	return pipelineRegistry.GetPipeline(inMaterial->GetShaderHash(), name);
}

Pipeline VulkanPassBase::CreateGraphicsPipeline(MaterialPtr inMaterial, PipelineLayout inLayout)
{
	PipelineShaderStageCreateInfo vertStageInfo;
	vertStageInfo.setStage(ShaderStageFlagBits::eVertex);
	vertStageInfo.setModule(inMaterial->GetVertexShader()->GetShaderModule());
	vertStageInfo.setPName("main");
	//vertStageInfo.setPSpecializationInfo(); spec info to set some constants

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
	viewport.setWidth(static_cast<float>(width));
	viewport.setHeight(static_cast<float>(height));
	viewport.setMinDepth(0.0f);
	viewport.setMaxDepth(1.0f);

	Rect2D scissor;
	scissor.setOffset(Offset2D(0, 0));
	scissor.setExtent(Extent2D{ width, height });

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

	//	PipelineDepthStencilStateCreateInfo depthStencilInfo;

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

	std::vector<DynamicState> dynamicStates = { /*DynamicState::eViewport,*/ DynamicState::eLineWidth };
	PipelineDynamicStateCreateInfo dynamicStateInfo;
	dynamicStateInfo.setDynamicStateCount(1);
	dynamicStateInfo.setPDynamicStates(dynamicStates.data());

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
	pipelineInfo.setPDynamicState(&dynamicStateInfo);
	pipelineInfo.setLayout(inLayout);
	pipelineInfo.setRenderPass(renderPass);
	pipelineInfo.setSubpass(0);
	pipelineInfo.setBasePipelineHandle(Pipeline());
	pipelineInfo.setBasePipelineIndex(-1);

	return vulkanDevice->GetDevice().createGraphicsPipeline(vulkanDevice->GetPipelineCache(), pipelineInfo);
}

DescriptorSetLayout VulkanPassBase::CreateDescriptorSetLayout(MaterialPtr inMaterial)
{
	DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
	descriptorSetLayoutInfo.setBindingCount(static_cast<uint32_t>(inMaterial->GetBindings().size()));
	descriptorSetLayoutInfo.setPBindings(inMaterial->GetBindings().data());

	return vulkanDevice->GetDevice().createDescriptorSetLayout(descriptorSetLayoutInfo);
}



