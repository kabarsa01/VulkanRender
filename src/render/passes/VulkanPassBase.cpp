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

VulkanPassBase::VulkanPassBase()
{

}

VulkanPassBase::~VulkanPassBase()
{

}

void VulkanPassBase::Create(VulkanDevice* inDevice)
{
	RendererPtr renderer = Engine::GetRendererInstance();
	width = renderer->GetWidth();
	height = renderer->GetHeight();

	vulkanDevice = inDevice;

	CreateRenderPass();
	CreateFramebufferResources();
	CreatePipelineLayout();
	CreateGraphicsPipeline();
	CreateDescriptorPool();
	AllocateDescriptorSets();

	frameDataBuffer.createInfo.setSize(sizeof(ShaderGlobalData));
	frameDataBuffer.createInfo.setUsage(BufferUsageFlagBits::eUniformBuffer);
	frameDataBuffer.createInfo.setSharingMode(SharingMode::eExclusive);
	frameDataBuffer.Create(vulkanDevice);
	frameDataBuffer.BindMemory(MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent);

	mvpBuffer.createInfo.setSize(sizeof(ObjectCommonData));
	mvpBuffer.createInfo.setUsage(BufferUsageFlagBits::eUniformBuffer);
	mvpBuffer.createInfo.setSharingMode(SharingMode::eExclusive);
	mvpBuffer.Create(vulkanDevice);
	mvpBuffer.BindMemory(MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent);

	UpdateDescriptorSets();
}

void VulkanPassBase::Destroy()
{
	Device& device = vulkanDevice->GetDevice();

	device.destroyDescriptorSetLayout(descriptorSetLayout);
	device.destroyDescriptorPool(descriptorPool);

	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipelineLayout);

	device.destroyRenderPass(renderPass);
	device.destroyFramebuffer(framebuffer);
	device.destroyImageView(imageView);
	image.Destroy();

	frameDataBuffer.Destroy();
	mvpBuffer.Destroy();
}

void VulkanPassBase::Draw(CommandBuffer* inCommandBuffer)
{
	UpdateUniformBuffers();

	ScenePtr scene = Engine::GetSceneInstance();
	CameraComponentPtr camComp = scene->GetSceneComponent<CameraComponent>();
	MeshComponentPtr meshComp = scene->GetSceneComponent<MeshComponent>();

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
	inCommandBuffer->bindPipeline(PipelineBindPoint::eGraphics, pipeline);
	inCommandBuffer->bindVertexBuffers(0, 1, &meshComp->meshData->GetVertexBuffer(), &offset);
	inCommandBuffer->bindIndexBuffer(meshComp->meshData->GetIndexBuffer(), 0, IndexType::eUint32);
	inCommandBuffer->bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSets, {});
	inCommandBuffer->drawIndexed(meshComp->meshData->GetIndexCount(), 1, 0, 0, 0);
	inCommandBuffer->endRenderPass();
}

void VulkanPassBase::CreateRenderPass()
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

	renderPass = device.createRenderPass(renderPassInfo);
}

void VulkanPassBase::CreateFramebufferResources()
{
	Device& device = vulkanDevice->GetDevice();
	RendererPtr renderer = Engine::GetRendererInstance();

	uint32_t queueFailyIndices[] = { vulkanDevice->GetPhysicalDevice().GetCachedQueueFamiliesIndices().graphicsFamily.value() };

	image.createInfo.setArrayLayers(1);
	image.createInfo.setFormat(Format::eR16G16B16A16Sfloat);
	image.createInfo.setImageType(ImageType::e2D);
	image.createInfo.setInitialLayout(ImageLayout::eUndefined);
	image.createInfo.setSamples(SampleCountFlagBits::e1);
	image.createInfo.setMipLevels(1);
	image.createInfo.setSharingMode(SharingMode::eExclusive);
	image.createInfo.setQueueFamilyIndexCount(1);
	image.createInfo.setPQueueFamilyIndices(queueFailyIndices);
	image.createInfo.setTiling(ImageTiling::eOptimal);
	image.createInfo.setFlags(ImageCreateFlags());
	image.createInfo.setExtent(Extent3D(renderer->GetWidth(), renderer->GetHeight(), 1));
	image.createInfo.setUsage(ImageUsageFlagBits::eColorAttachment | ImageUsageFlagBits::eSampled | ImageUsageFlagBits::eInputAttachment);
	image.Create(vulkanDevice);
	image.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);

	ComponentMapping compMapping;
	compMapping.setR(ComponentSwizzle::eIdentity);
	compMapping.setG(ComponentSwizzle::eIdentity);
	compMapping.setB(ComponentSwizzle::eIdentity);
	compMapping.setA(ComponentSwizzle::eIdentity);

	ImageSubresourceRange imageSubresRange;
	imageSubresRange.setBaseArrayLayer(0);
	imageSubresRange.setAspectMask(ImageAspectFlagBits::eColor);
	imageSubresRange.setBaseMipLevel(0);
	imageSubresRange.setLayerCount(1);
	imageSubresRange.setLevelCount(1);

	ImageViewCreateInfo imageViewInfo;
	imageViewInfo.setComponents(compMapping);
	imageViewInfo.setFormat(Format::eR16G16B16A16Sfloat);
	imageViewInfo.setImage(image);
	imageViewInfo.setSubresourceRange(imageSubresRange);
	imageViewInfo.setViewType(ImageViewType::e2D);
	imageView = device.createImageView(imageViewInfo);

	FramebufferCreateInfo framebufferInfo;
	framebufferInfo.setRenderPass(renderPass);
	framebufferInfo.setAttachmentCount(1);
	framebufferInfo.setPAttachments(&imageView);
	framebufferInfo.setWidth(renderer->GetWidth());
	framebufferInfo.setHeight(renderer->GetHeight());
	framebufferInfo.setLayers(1);

	framebuffer = device.createFramebuffer(framebufferInfo);
}

void VulkanPassBase::CreatePipelineLayout()
{
	Device& device = vulkanDevice->GetDevice();

	DescriptorSetLayoutBinding globalsLayoutBinding;
	globalsLayoutBinding.setBinding(0);
	globalsLayoutBinding.setDescriptorType(DescriptorType::eUniformBuffer);
	globalsLayoutBinding.setDescriptorCount(1);
	globalsLayoutBinding.setStageFlags(ShaderStageFlagBits::eAllGraphics);
	DescriptorSetLayoutBinding mvpLayoutBinding;
	mvpLayoutBinding.setBinding(2);
	mvpLayoutBinding.setDescriptorType(DescriptorType::eUniformBuffer);
	mvpLayoutBinding.setDescriptorCount(1);
	mvpLayoutBinding.setStageFlags(ShaderStageFlagBits::eAllGraphics);
	DescriptorSetLayoutBinding samplerLayoutBinding;
	samplerLayoutBinding.setBinding(1);
	samplerLayoutBinding.setDescriptorType(DescriptorType::eSampler);
	samplerLayoutBinding.setDescriptorCount(1);
	samplerLayoutBinding.setStageFlags(ShaderStageFlagBits::eFragment);

	DescriptorSetLayoutBinding setLayoutBindings[] = { globalsLayoutBinding, mvpLayoutBinding, samplerLayoutBinding };
	DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
	descriptorSetLayoutInfo.setBindingCount(3);
	descriptorSetLayoutInfo.setPBindings(setLayoutBindings);

	descriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutInfo);

	PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setFlags(PipelineLayoutCreateFlags());
	pipelineLayoutInfo.setSetLayoutCount(1);
	pipelineLayoutInfo.setPSetLayouts(&descriptorSetLayout);

	pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
}

void VulkanPassBase::CreateGraphicsPipeline()
{
	Device& device = vulkanDevice->GetDevice();

	Shader vertShader;
	vertShader.Load("content/shaders/BasePassVert.spv");
	Shader fragShader;
	fragShader.Load("content/shaders/BasePassFrag.spv");

	VulkanShaderModule vertShaderModule(vertShader);
	VulkanShaderModule fragShaderModule(fragShader);

	PipelineShaderStageCreateInfo vertStageInfo;
	vertStageInfo.setStage(ShaderStageFlagBits::eVertex);
	vertStageInfo.setModule(vertShaderModule.GetShaderModule());
	vertStageInfo.setPName("main");
	//vertStageInfo.setPSpecializationInfo(); spec info to set some constants

	PipelineShaderStageCreateInfo fragStageInfo;
	fragStageInfo.setStage(ShaderStageFlagBits::eFragment);
	fragStageInfo.setModule(fragShaderModule.GetShaderModule());
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
	pipelineInfo.setLayout(pipelineLayout);
	pipelineInfo.setRenderPass(renderPass);
	pipelineInfo.setSubpass(0);
	pipelineInfo.setBasePipelineHandle(Pipeline());
	pipelineInfo.setBasePipelineIndex(-1);

	pipeline = device.createGraphicsPipeline(vulkanDevice->GetPipelineCache(), pipelineInfo);
}

void VulkanPassBase::CreateDescriptorPool()
{
	DescriptorPoolSize uniformPoolSize;
	uniformPoolSize.setDescriptorCount(32);
	uniformPoolSize.setType(DescriptorType::eUniformBuffer);
	DescriptorPoolSize samplerPoolSize;
	samplerPoolSize.setDescriptorCount(16);
	samplerPoolSize.setType(DescriptorType::eSampler);

	DescriptorPoolSize poolSizes[] = { uniformPoolSize, samplerPoolSize };
	DescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.setPoolSizeCount(2);
	descPoolInfo.setPPoolSizes(poolSizes);
	descPoolInfo.setMaxSets(8);

	descriptorPool = vulkanDevice->GetDevice().createDescriptorPool(descPoolInfo);
}

// this should be split in actual allocation and update for desc sets
void VulkanPassBase::AllocateDescriptorSets()
{
	std::vector<DescriptorSetLayout> layouts = { descriptorSetLayout };
	DescriptorSetAllocateInfo descSetAllocInfo;
	descSetAllocInfo.setDescriptorPool(descriptorPool);
	descSetAllocInfo.setDescriptorSetCount(static_cast<uint32_t>(layouts.size()));
	descSetAllocInfo.setPSetLayouts(layouts.data());

	descriptorSets = vulkanDevice->GetDevice().allocateDescriptorSets(descSetAllocInfo);
}

// this should be about updating buffers
void VulkanPassBase::UpdateDescriptorSets()
{

	for (uint32_t index = 0; index < descriptorSets.size(); index++)
	{
		DescriptorBufferInfo descBufferInfo;
		descBufferInfo.setBuffer(mvpBuffer);
		descBufferInfo.setOffset(0);
		descBufferInfo.setRange(sizeof(ObjectCommonData));// VK_WHOLE_SIZE;

		WriteDescriptorSet mvpWriteDescSet;
		mvpWriteDescSet.setDstSet(descriptorSets[index]);
		mvpWriteDescSet.setDstBinding(0);
		mvpWriteDescSet.setDstArrayElement(0); // globals is 0, uniform buffer is 1
		mvpWriteDescSet.setDescriptorCount(1);
		mvpWriteDescSet.setDescriptorType(DescriptorType::eUniformBuffer);
		mvpWriteDescSet.setPBufferInfo(&descBufferInfo);

		DescriptorBufferInfo frameDataInfo;
		frameDataInfo.setBuffer(frameDataBuffer);
		frameDataInfo.setOffset(0);
		frameDataInfo.setRange(sizeof(ShaderGlobalData));// VK_WHOLE_SIZE;

		WriteDescriptorSet frameDataWriteDescSet;
		frameDataWriteDescSet.setDstSet(descriptorSets[index]);
		frameDataWriteDescSet.setDstBinding(2);
		frameDataWriteDescSet.setDstArrayElement(0); // globals is 0, uniform buffer is 1
		frameDataWriteDescSet.setDescriptorCount(1);
		frameDataWriteDescSet.setDescriptorType(DescriptorType::eUniformBuffer);
		frameDataWriteDescSet.setPBufferInfo(&frameDataInfo);

		WriteDescriptorSet writes[] = { frameDataWriteDescSet, mvpWriteDescSet };
		vulkanDevice->GetDevice().updateDescriptorSets(2, writes, 0, nullptr);
	}
}

void VulkanPassBase::UpdateUniformBuffers()
{
	ScenePtr scene = Engine::GetSceneInstance();
	CameraComponentPtr camComp = scene->GetSceneComponent<CameraComponent>();
	MeshComponentPtr meshComp = scene->GetSceneComponent<MeshComponent>();

	ObjectCommonData ubo;
	ubo.model = meshComp->GetParent()->transform.GetMatrix();
	ubo.view = camComp->CalculateViewMatrix();
	ubo.proj = camComp->CalculateProjectionMatrix();

	{
		MemoryRecord& memRec = mvpBuffer.GetMemoryRecord();
		memRec.pos.memory.MapCopyUnmap(MemoryMapFlags(), memRec.pos.offset, sizeof(ObjectCommonData), &ubo, 0, sizeof(ObjectCommonData));
	}

	ShaderGlobalData frameData;
	frameData.view = ubo.view;
	frameData.proj = ubo.proj;

	{
		MemoryRecord& memRec = frameDataBuffer.GetMemoryRecord();
		memRec.pos.memory.MapCopyUnmap(MemoryMapFlags(), memRec.pos.offset, sizeof(ShaderGlobalData), &frameData, 0, sizeof(ShaderGlobalData));
	}
}

