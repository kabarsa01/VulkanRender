#include "render/Renderer.h"
#include <iostream>
#include <map>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3native.h>
#include "glm/gtc/matrix_transform.hpp"
#include <chrono>
#include "core/Engine.h"
#include <vector>
#include <algorithm>
#include "shader/Shader.h"
#include "shader/VulkanShaderModule.h"
#include <array>
#include "scene/camera/CameraComponent.h"
#include "scene/mesh/MeshComponent.h"
#include "scene/SceneObjectComponent.h"
#include "scene/Transform.h"
#include "scene/SceneObjectBase.h"
#include "DataStructures.h"
#include "TransferList.h"
#include "data/DataManager.h"
#include "passes/VulkanPassBase.h"
#include "PerFrameData.h"

const std::vector<Vertex> verticesTest = {
	{{0.0f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
};

const std::vector<Vertex> verticesToIndex = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint32_t> indices = {
	0, 1, 2, 2, 3, 0
};

Renderer::Renderer()
{
}

Renderer::~Renderer()
{

}

void Renderer::OnInitialize()
{

}

void Renderer::Init()
{
	GLFWwindow* window = Engine::GetInstance()->GetGlfwWindow();
	glfwGetFramebufferSize(window, &width, &height);

	HWND hWnd = glfwGetWin32Window(Engine::GetInstance()->GetGlfwWindow());
	device.Create("VulkanRenderer", "VulkanEngine", enableValidationLayers, hWnd);
	swapChain.Create(&device, 2);
	swapChain.CreateForResolution(width, height);
	commandBuffers.Create(&device, 2, 1);

	CreateDescriptorPool();

	perFrameData = new PerFrameData();
	perFrameData->Create(&device, descriptorPool);
	basePass = new VulkanPassBase();
	basePass->Create(&device);

	CreateDescriptorSetLayout();
	CreateGraphicsPipeline(swapChain.GetRenderPass(), swapChain.GetExtent());

	CreateUniformBuffers();
	CreateImageAndSampler();
	CreateDescriptorSets();
}

void Renderer::RenderFrame()
{
	if (framebufferResized)
	{
		OnResolutionChange();
		framebufferResized = false;
		return;
	}

	bool outdated = false;
	uint32_t imageIndex = swapChain.AcquireNextImage(outdated);
	if (outdated)
	{
		OnResolutionChange();
		return;
	}

	UpdateUniformBuffer();
	perFrameData->UpdateBufferData();

	CommandBuffer& cmdBuffer = commandBuffers.GetNextForPool(imageIndex);

	CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(CommandBufferUsageFlagBits::eSimultaneousUse);
	beginInfo.setPInheritanceInfo(nullptr);

	// begin command buffer record
	cmdBuffer.begin(beginInfo);
	// copy new data
	TransferResources(cmdBuffer, device.GetPhysicalDevice().GetCachedQueueFamiliesIndices().graphicsFamily.value());
	// render passes
	basePass->Draw(&cmdBuffer);
	UpdateCommandBuffer(cmdBuffer, swapChain.GetRenderPass(), swapChain.GetFramebuffer(imageIndex), pipeline, pipelineLayout);
	// end commands recording
	cmdBuffer.end();

	SubmitInfo submitInfo;
	Semaphore waitSemaphores[] = { swapChain.GetImageAvailableSemaphore() };
	PipelineStageFlags waitStages[] = { PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.setWaitSemaphoreCount(1);
	submitInfo.setPWaitSemaphores(waitSemaphores);
	submitInfo.setPWaitDstStageMask(waitStages);
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&cmdBuffer);

	Semaphore signalSemaphores[] = { swapChain.GetRenderingFinishedSemaphore() };
	submitInfo.setSignalSemaphoreCount(1);
	submitInfo.setPSignalSemaphores(signalSemaphores);

	ArrayProxy<const SubmitInfo> submitInfoArray(1, &submitInfo);
	device.GetGraphicsQueue().submit(submitInfoArray, swapChain.GetGraphicsQueueFence());

	if (!swapChain.Present())
	{
		OnResolutionChange();
	}
	swapChain.WaitForPresentQueue();
}

void Renderer::WaitForDevice()
{
	device.GetDevice().waitIdle();
}

void Renderer::Cleanup()
{
	WaitForDevice();

	basePass->Destroy();
	delete basePass;

	ScenePtr scene = Engine::GetSceneInstance();
	MeshComponentPtr meshComp = scene->GetSceneComponent<MeshComponent>();
	meshComp->meshData->DestroyBuffer();

	uniformBuffer.Destroy();

	perFrameData->Destroy();
	delete perFrameData;

	device.GetDevice().destroyDescriptorSetLayout(descriptorSetLayout);
	device.GetDevice().destroyDescriptorPool(descriptorPool);
	// destroying pipelines
	DestroyGraphicsPipeline();
	PipelineRegistry::GetInstance()->DestroyPipelines(&device);
	
	device.GetDevice().destroySampler(sampler);

	commandBuffers.Destroy();
	swapChain.Destroy();
	device.Destroy();
}

void Renderer::SetResolution(int inWidth, int inHeight)
{
	//width = inWidth;
	//height = inHeight;

	framebufferResized = true;
}

int Renderer::GetWidth() const
{
	return width;
}

int Renderer::GetHeight() const
{
	return height;
}

VulkanDevice& Renderer::GetVulkanDevice()
{
	return device;
}

Device& Renderer::GetDevice()
{
	return device.GetDevice();
}

VulkanSwapChain& Renderer::GetSwapChain()
{
	return swapChain;
}

VulkanCommandBuffers& Renderer::GetCommandBuffers()
{
	return commandBuffers;
}

Queue Renderer::GetGraphicsQueue()
{
	return device.GetGraphicsQueue();
}

void Renderer::CreateDescriptorSetLayout()
{
	DescriptorSetLayoutBinding uniforBufferBinding;
	uniforBufferBinding.setBinding(0);
	uniforBufferBinding.setDescriptorType(DescriptorType::eUniformBuffer);
	uniforBufferBinding.setDescriptorCount(1);
	uniforBufferBinding.setStageFlags(ShaderStageFlagBits::eVertex);
	uniforBufferBinding.setPImmutableSamplers(nullptr);
	DescriptorSetLayoutBinding samplerBinding;
	samplerBinding.setBinding(1);
	samplerBinding.setDescriptorType(DescriptorType::eCombinedImageSampler);
	samplerBinding.setDescriptorCount(1);
	samplerBinding.setStageFlags(ShaderStageFlagBits::eFragment);
	samplerBinding.setPImmutableSamplers(nullptr);
	DescriptorSetLayoutBinding inputAttachmentBinding;
	inputAttachmentBinding.setBinding(2);
	inputAttachmentBinding.setDescriptorType(DescriptorType::eInputAttachment);
	inputAttachmentBinding.setDescriptorCount(1);
	inputAttachmentBinding.setStageFlags(ShaderStageFlagBits::eFragment);
	inputAttachmentBinding.setPImmutableSamplers(nullptr);

	DescriptorSetLayoutBinding bindings[] = { uniforBufferBinding, samplerBinding, inputAttachmentBinding };
	DescriptorSetLayoutCreateInfo descSetLayoutInfo;
	descSetLayoutInfo.setBindingCount(3);
	descSetLayoutInfo.setPBindings(bindings);

	descriptorSetLayout = device.GetDevice().createDescriptorSetLayout(descSetLayoutInfo);
}

void Renderer::CreateGraphicsPipeline(RenderPass& inRenderPass, Extent2D inExtent)
{
	DataManager* DM = DataManager::GetInstance();
	ShaderPtr vertShader = DM->RequestResourceByType<Shader>(std::string("content/shaders/BasicVert.spv"));
	vertShader->Load();
	ShaderPtr fragShader = DM->RequestResourceByType<Shader>(std::string("content/shaders/BasicFrag.spv"));
	fragShader->Load();

	PipelineShaderStageCreateInfo vertStageInfo;
	vertStageInfo.setStage(ShaderStageFlagBits::eVertex);
	vertStageInfo.setModule(vertShader->GetShaderModule());
	vertStageInfo.setPName("main");
	//vertStageInfo.setPSpecializationInfo(); spec info to set some constants

	PipelineShaderStageCreateInfo fragStageInfo;
	fragStageInfo.setStage(ShaderStageFlagBits::eFragment);
	fragStageInfo.setModule(fragShader->GetShaderModule());
	fragStageInfo.setPName("main");

	std::vector<PipelineShaderStageCreateInfo> shaderStageInfoArray = { vertStageInfo, fragStageInfo };

	VertexInputBindingDescription bindingDesc = MeshData::GetBindingDescription(0);
	std::array<VertexInputAttributeDescription, 5> attributeDesc = Vertex::GetAttributeDescriptions(0);
	PipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.setVertexBindingDescriptionCount(1);
	vertexInputInfo.setPVertexBindingDescriptions(&bindingDesc);
	vertexInputInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>( attributeDesc.size() ));
	vertexInputInfo.setPVertexAttributeDescriptions(attributeDesc.data());

	PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	inputAssemblyInfo.setTopology(PrimitiveTopology::eTriangleList);
	inputAssemblyInfo.setPrimitiveRestartEnable(VK_FALSE);

//	Viewport viewport;
	viewport.setX(0.0f);
	viewport.setY(0.0f);
	viewport.setWidth((float)inExtent.width);
	viewport.setHeight((float)inExtent.height);
	viewport.setMinDepth(0.0f);
	viewport.setMaxDepth(1.0f);

	Rect2D scissor;
	scissor.setOffset(Offset2D(0, 0));
	scissor.setExtent(inExtent);

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
	colorBlendInfo.setBlendConstants( { 0.0f, 0.0f, 0.0f, 0.0f } );

	std::vector<DynamicState> dynamicStates = { DynamicState::eViewport, DynamicState::eLineWidth };
	PipelineDynamicStateCreateInfo dynamicStateInfo;
	dynamicStateInfo.setDynamicStateCount(2); // 2
	dynamicStateInfo.setPDynamicStates(dynamicStates.data());

	PipelineLayoutCreateInfo layoutInfo;
	layoutInfo.setSetLayoutCount(1);
	DescriptorSetLayout layoutBindings[] = { descriptorSetLayout };
	layoutInfo.setPSetLayouts(layoutBindings);
	layoutInfo.setPushConstantRangeCount(0);
	layoutInfo.setPPushConstantRanges(nullptr);

	pipelineLayout = device.GetDevice().createPipelineLayout(layoutInfo);

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
	pipelineInfo.setRenderPass(inRenderPass);
	pipelineInfo.setSubpass(0);
	pipelineInfo.setBasePipelineHandle(Pipeline());
	pipelineInfo.setBasePipelineIndex(-1);

	pipeline = device.GetDevice().createGraphicsPipeline(device.GetPipelineCache(), pipelineInfo);
}

void Renderer::DestroyGraphicsPipeline()
{
	device.GetDevice().destroyPipeline(pipeline);
	device.GetDevice().destroyPipelineLayout(pipelineLayout);
}

void Renderer::UpdateCommandBuffer(CommandBuffer& inCommandBuffer, RenderPass& inRenderPass, Framebuffer& inFrameBuffer, Pipeline& inPipeline, PipelineLayout& inPipelineLayout)
{
	ScenePtr scene = Engine::GetSceneInstance();
	CameraComponentPtr camComp = scene->GetSceneComponent<CameraComponent>();
	MeshComponentPtr meshComp = scene->GetSceneComponent<MeshComponent>();
	MeshDataPtr meshData = MeshData::FullscreenQuad();

	ImageMemoryBarrier barrier;
	barrier.setOldLayout(ImageLayout::eUndefined);
	barrier.setNewLayout(ImageLayout::eShaderReadOnlyOptimal);
	barrier.setImage(basePass->GetImage());
	barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	barrier.subresourceRange.setAspectMask(ImageAspectFlagBits::eColor);
	barrier.subresourceRange.setBaseMipLevel(0);
	barrier.subresourceRange.setLevelCount(1);
	barrier.subresourceRange.setBaseArrayLayer(0);
	barrier.subresourceRange.setLayerCount(1);
	barrier.setSrcAccessMask(AccessFlagBits::eColorAttachmentWrite);
	barrier.setDstAccessMask(AccessFlagBits::eShaderRead);

	inCommandBuffer.pipelineBarrier(PipelineStageFlagBits::eColorAttachmentOutput, PipelineStageFlagBits::eFragmentShader, DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

	ClearValue clearValue;
	clearValue.setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));

	RenderPassBeginInfo passBeginInfo;
	passBeginInfo.setRenderPass(inRenderPass);
	passBeginInfo.setFramebuffer(inFrameBuffer);
	passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), swapChain.GetExtent()));
	passBeginInfo.setClearValueCount(1);
	passBeginInfo.setPClearValues(&clearValue);

	DeviceSize offset = 0;
	inCommandBuffer.beginRenderPass(passBeginInfo, SubpassContents::eInline);
	inCommandBuffer.bindPipeline(PipelineBindPoint::eGraphics, inPipeline);
	inCommandBuffer.setViewport(0, 1, &viewport);
	inCommandBuffer.bindVertexBuffers(0, 1, &meshData->GetVertexBuffer().GetBuffer(), &offset);
	inCommandBuffer.bindIndexBuffer(meshData->GetIndexBuffer().GetBuffer(), 0, IndexType::eUint32);
	inCommandBuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, inPipelineLayout, 0, descriptorSets, {});
	inCommandBuffer.drawIndexed(meshData->GetIndexCount(), 1, 0, 0, 0);
	inCommandBuffer.endRenderPass();
}

void Renderer::CreateUniformBuffers()
{
	uniformBuffer.createInfo.setSize(sizeof(ObjectMVPData));
	uniformBuffer.createInfo.setUsage(BufferUsageFlagBits::eUniformBuffer);
	uniformBuffer.createInfo.setSharingMode(SharingMode::eExclusive);
	uniformBuffer.Create(&device);
	uniformBuffer.BindMemory(MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent);
}

void Renderer::CreateDescriptorPool()
{
	DescriptorPoolSize uniformSize;
	uniformSize.setDescriptorCount(128);
	uniformSize.setType(DescriptorType::eUniformBuffer);
	DescriptorPoolSize samplerSize;
	samplerSize.setDescriptorCount(16);
	samplerSize.setType(DescriptorType::eSampler);
	DescriptorPoolSize imageSize;
	imageSize.setDescriptorCount(128);
	imageSize.setType(DescriptorType::eSampledImage);

	std::array<DescriptorPoolSize, 3> sizes = { uniformSize, samplerSize, imageSize };
	DescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.setPoolSizeCount(static_cast<uint32_t>( sizes.size() ));
	descPoolInfo.setPPoolSizes(sizes.data());
	descPoolInfo.setMaxSets(128);

	descriptorPool = device.GetDevice().createDescriptorPool(descPoolInfo);
}

void Renderer::CreateDescriptorSets()
{
	std::vector<DescriptorSetLayout> layouts = {descriptorSetLayout};
	DescriptorSetAllocateInfo descSetAllocInfo;
	descSetAllocInfo.setDescriptorPool(descriptorPool);
	descSetAllocInfo.setDescriptorSetCount(static_cast<uint32_t>(layouts.size()));
	descSetAllocInfo.setPSetLayouts(layouts.data());

	descriptorSets = device.GetDevice().allocateDescriptorSets(descSetAllocInfo);

	for (uint32_t index = 0; index < descriptorSets.size(); index++)
	{
		DescriptorBufferInfo descBufferInfo;
		descBufferInfo.setBuffer(uniformBuffer);
		descBufferInfo.setOffset(0);
		descBufferInfo.setRange(sizeof(ObjectMVPData));// VK_WHOLE_SIZE;

		WriteDescriptorSet uniformBufferWrite;
		uniformBufferWrite.setDstSet(descriptorSets[index]);
		uniformBufferWrite.setDstBinding(0);
		uniformBufferWrite.setDstArrayElement(0);
		uniformBufferWrite.setDescriptorCount(1);
		uniformBufferWrite.setDescriptorType(DescriptorType::eUniformBuffer);
		uniformBufferWrite.setPBufferInfo(&descBufferInfo);

		DescriptorImageInfo samplerInfo;
		samplerInfo.setSampler(sampler);
		samplerInfo.setImageView(basePass->GetImageView());
		samplerInfo.setImageLayout(ImageLayout::eShaderReadOnlyOptimal);//eShaderReadOnlyOptimal);

		WriteDescriptorSet samplerWrite;
		samplerWrite.setDstSet(descriptorSets[index]);
		samplerWrite.setDstBinding(1);
		samplerWrite.setDstArrayElement(0);
		samplerWrite.setDescriptorCount(1);
		samplerWrite.setDescriptorType(DescriptorType::eCombinedImageSampler);
		samplerWrite.setPImageInfo(&samplerInfo);

		WriteDescriptorSet writes[] = { uniformBufferWrite, samplerWrite };
		device.GetDevice().updateDescriptorSets(2, writes, 0, nullptr);
	}
}

void Renderer::OnResolutionChange()
{
	device.GetDevice().waitIdle();

	GLFWwindow* window = Engine::GetInstance()->GetGlfwWindow();
	glfwGetFramebufferSize(window, &width, &height);

	DestroyGraphicsPipeline();
	swapChain.DestroyForResolution();

	swapChain.CreateForResolution(width, height);
	CreateGraphicsPipeline(swapChain.GetRenderPass(), swapChain.GetExtent());
}

void Renderer::CreateImageAndSampler()
{
	uint32_t queueFamilyIndices[] = { device.GetPhysicalDevice().GetCachedQueueFamiliesIndices().graphicsFamily.value() };

	//image.createInfo.setArrayLayers(1);
	//image.createInfo.setFormat(Format::eR8G8B8A8Srgb);
	//image.createInfo.setImageType(ImageType::e2D);
	//image.createInfo.setInitialLayout(ImageLayout::eUndefined);
	//image.createInfo.setSamples(SampleCountFlagBits::e1);
	//image.createInfo.setMipLevels(1);
	//image.createInfo.setSharingMode(SharingMode::eExclusive);
	//image.createInfo.setQueueFamilyIndexCount(1);
	//image.createInfo.setPQueueFamilyIndices(queueFailyIndices);
	//image.createInfo.setTiling(ImageTiling::eOptimal);
	//image.createInfo.setFlags(ImageCreateFlags());
	//image.createInfo.setExtent(Extent3D(width, height, 1));
	//image.createInfo.setUsage(ImageUsageFlagBits::eSampled);
	//image.Create(&device);
	//image.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);

	//ComponentMapping compMapping;
	//compMapping.setR(ComponentSwizzle::eIdentity);
	//compMapping.setG(ComponentSwizzle::eIdentity);
	//compMapping.setB(ComponentSwizzle::eIdentity);
	//compMapping.setA(ComponentSwizzle::eIdentity);

	//ImageSubresourceRange imageSubresRange;
	//imageSubresRange.setBaseArrayLayer(0);
	//imageSubresRange.setAspectMask(ImageAspectFlagBits::eColor);
	//imageSubresRange.setBaseMipLevel(0);
	//imageSubresRange.setLayerCount(1);
	//imageSubresRange.setLevelCount(1);

	//ImageViewCreateInfo imageViewInfo;
	//imageViewInfo.setComponents(compMapping);
	//imageViewInfo.setFormat(Format::eR8G8B8A8Srgb);
	//imageViewInfo.setImage(image);
	//imageViewInfo.setSubresourceRange(imageSubresRange);
	//imageViewInfo.setViewType(ImageViewType::e2D);
	//imageView = device.GetDevice().createImageView(imageViewInfo);

	SamplerCreateInfo samplerInfo;
	samplerInfo.setAddressModeU(SamplerAddressMode::eRepeat);
	samplerInfo.setAddressModeV(SamplerAddressMode::eRepeat);
	samplerInfo.setAddressModeW(SamplerAddressMode::eRepeat);
	samplerInfo.setAnisotropyEnable(VK_FALSE);
	samplerInfo.setBorderColor(BorderColor::eIntOpaqueBlack);
	samplerInfo.setCompareEnable(VK_FALSE);
	samplerInfo.setMagFilter(Filter::eLinear);
	samplerInfo.setMaxAnisotropy(2);
	samplerInfo.setMaxLod(0);
	samplerInfo.setMinFilter(Filter::eLinear);
	samplerInfo.setMinLod(0);
	samplerInfo.setMipLodBias(0.0f);
	samplerInfo.setMipmapMode(SamplerMipmapMode::eLinear);
	samplerInfo.setUnnormalizedCoordinates(VK_FALSE);

	sampler = device.GetDevice().createSampler(samplerInfo);
}

void Renderer::UpdateUniformBuffer()
{
	ScenePtr scene = Engine::GetSceneInstance();
	CameraComponentPtr camComp = scene->GetSceneComponent<CameraComponent>();
	MeshComponentPtr meshComp = scene->GetSceneComponent<MeshComponent>();

	ObjectMVPData ubo;

	glm::vec3 scale = meshComp->GetParent()->transform.GetScale();
	meshComp->GetParent()->transform.SetScale({ 10.0f, 10.0f, 10.0f });
	ubo.model = meshComp->GetParent()->transform.GetMatrix();
	meshComp->GetParent()->transform.SetScale(scale);

	ubo.view = camComp->CalculateViewMatrix();
	ubo.proj = camComp->CalculateProjectionMatrix(); 

	MemoryRecord& memRec = uniformBuffer.GetMemoryRecord();
	memRec.pos.memory.MapCopyUnmap(MemoryMapFlags(), memRec.pos.offset, sizeof(ObjectMVPData), &ubo, 0, sizeof(ObjectMVPData));
}

void Renderer::TransferResources(CommandBuffer& inCmdBuffer, uint32_t inQueueFamilyIndex)
{
	TransferList* TL = TransferList::GetInstance();

	// get new resources to copy
	std::vector<VulkanBuffer*> buffers = TL->GetBuffers();
	std::vector<VulkanImage*> images = TL->GetImages();
	TL->ClearBuffers();
	TL->ClearImages();

	if ( (buffers.size() == 0) && (images.size() == 0) )
	{
		return;
	}

	// buffers
	std::vector<BufferMemoryBarrier> buffersTransferBarriers;
	for (VulkanBuffer* buffer : buffers)
	{
		inCmdBuffer.copyBuffer(*buffer->CreateStagingBuffer(), *buffer, 1, &buffer->CreateBufferCopy());
		buffersTransferBarriers.push_back(buffer->CreateMemoryBarrier(
			VK_QUEUE_FAMILY_IGNORED, 
			VK_QUEUE_FAMILY_IGNORED, 
			AccessFlagBits::eTransferWrite, 
			AccessFlagBits::eVertexAttributeRead));
	}

	// images
	// prepare memory barriers first
	std::vector<ImageMemoryBarrier> beforeTransferBarriers;
	std::vector<ImageMemoryBarrier> afterTransferBarriers;
	beforeTransferBarriers.resize(images.size());
	afterTransferBarriers.resize(images.size());
	for (uint32_t index = 0; index < images.size(); index++)
	{
		beforeTransferBarriers[index] = images[index]->CreateLayoutBarrier(
			ImageLayout::eUndefined,
			ImageLayout::eTransferDstOptimal,
			AccessFlagBits::eHostWrite,
			AccessFlagBits::eTransferWrite | AccessFlagBits::eTransferRead,
			ImageAspectFlagBits::eColor,
			0, 1, 0, 1);

		afterTransferBarriers[index] = images[index]->CreateLayoutBarrier(
			ImageLayout::eTransferDstOptimal,
			ImageLayout::eShaderReadOnlyOptimal,
			AccessFlagBits::eTransferWrite,
			AccessFlagBits::eShaderRead,
			ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
	}

	inCmdBuffer.pipelineBarrier(
		PipelineStageFlagBits::eHost,
		PipelineStageFlagBits::eTransfer,
		DependencyFlags(),
		0, nullptr, 0, nullptr,
		static_cast<uint32_t>( beforeTransferBarriers.size() ), 
		beforeTransferBarriers.data());

	//submit copy
	for (VulkanImage* image : images)
	{
		// copy
		inCmdBuffer.copyBufferToImage(
			*image->CreateStagingBuffer(SharingMode::eExclusive, inQueueFamilyIndex), 
			*image, ImageLayout::eTransferDstOptimal, 
			1, &image->CreateBufferImageCopy());
	}

	// final barriers for buffers and images
	inCmdBuffer.pipelineBarrier(
		PipelineStageFlagBits::eTransfer,
		PipelineStageFlagBits::eVertexInput | PipelineStageFlagBits::eVertexShader,
		DependencyFlags(),
		0, nullptr, 
		static_cast<uint32_t>( buffersTransferBarriers.size() ),
		buffersTransferBarriers.data(),
		static_cast<uint32_t>( afterTransferBarriers.size() ),
		afterTransferBarriers.data());
}

