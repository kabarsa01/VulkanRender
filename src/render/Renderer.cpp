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

	CreateDescriptorSetLayout();
	CreateGraphicsPipeline(swapChain.GetRenderPass(), swapChain.GetExtent());

	CreateUniformBuffers();
	CreateDescriptorPool();
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

	CommandBuffer& cmdBuffer = commandBuffers.GetNextForPool(imageIndex);

	UpdateCommandBuffer(cmdBuffer, swapChain.GetRenderPass(), swapChain.GetFramebuffer(imageIndex), pipeline, pipelineLayout);

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
		return;
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

	ScenePtr scene = Engine::GetSceneInstance();
	MeshComponentPtr meshComp = scene->GetSceneComponent<MeshComponent>();
	meshComp->meshData->DestroyBuffer();

	uniformBuffer.Destroy();

	device.GetDevice().destroyDescriptorSetLayout(descriptorSetLayout);
	device.GetDevice().destroyDescriptorPool(descriptorPool);
	// destroying pipelines
	DestroyGraphicsPipeline();
	
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

//int Renderer::GetWidth() const
//{
//	return width;
//}
//
//int Renderer::GetHeight() const
//{
//	return height;
//}

VulkanDevice& Renderer::GetVulkanDevice()
{
	return device;
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
	DescriptorSetLayoutBinding descLayoutBinding;
	descLayoutBinding.setBinding(0);
	descLayoutBinding.setDescriptorType(DescriptorType::eUniformBuffer);
	descLayoutBinding.setDescriptorCount(1);
	descLayoutBinding.setStageFlags(ShaderStageFlagBits::eVertex);
	descLayoutBinding.setPImmutableSamplers(nullptr);

	DescriptorSetLayoutCreateInfo descSetLayoutInfo;
	descSetLayoutInfo.setBindingCount(1);
	descSetLayoutInfo.setPBindings(&descLayoutBinding);

	descriptorSetLayout = device.GetDevice().createDescriptorSetLayout(descSetLayoutInfo);
}

void Renderer::CreateGraphicsPipeline(RenderPass& inRenderPass, Extent2D inExtent)
{
	Shader vertShader;
	vertShader.Load("content/shaders/BasicVert.spv");
	Shader fragShader;
	fragShader.Load("content/shaders/BasicFrag.spv");

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
	colorBlendInfo.setBlendConstants( { 0.0f, 0.0f, 0.0f, 0.0f } );

	std::vector<DynamicState> dynamicStates = { DynamicState::eViewport, DynamicState::eLineWidth };
	PipelineDynamicStateCreateInfo dynamicStateInfo;
	dynamicStateInfo.setDynamicStateCount(2);
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

	CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(CommandBufferUsageFlagBits::eSimultaneousUse);
	beginInfo.setPInheritanceInfo(nullptr);

	inCommandBuffer.begin(beginInfo);

	ClearValue clearValue;
	clearValue.setColor(ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f })));

	RenderPassBeginInfo passBeginInfo;
	passBeginInfo.setRenderPass(inRenderPass);
	passBeginInfo.setFramebuffer(inFrameBuffer);
	passBeginInfo.setRenderArea(Rect2D(Offset2D(0, 0), swapChain.GetExtent()));
	passBeginInfo.setClearValueCount(1);
	passBeginInfo.setPClearValues(&clearValue);

	DeviceSize offset = 0;
	inCommandBuffer.setViewport(0, 1, &viewport);
	inCommandBuffer.beginRenderPass(passBeginInfo, SubpassContents::eInline);
	inCommandBuffer.bindPipeline(PipelineBindPoint::eGraphics, inPipeline);
	inCommandBuffer.bindVertexBuffers(0, 1, &meshComp->meshData->GetVertexBuffer(), &offset);
	inCommandBuffer.bindIndexBuffer(meshComp->meshData->GetIndexBuffer(), 0, IndexType::eUint32);
	inCommandBuffer.bindDescriptorSets(PipelineBindPoint::eGraphics, inPipelineLayout, 0, descriptorSets, {});
	inCommandBuffer.drawIndexed(meshComp->meshData->GetIndexCount(), 1, 0, 0, 0);
	inCommandBuffer.endRenderPass();
	inCommandBuffer.end();
}

void Renderer::CreateUniformBuffers()
{
	uniformBuffer.createInfo.setSize(sizeof(UniformBufferObject));
	uniformBuffer.createInfo.setUsage(BufferUsageFlagBits::eUniformBuffer);
	uniformBuffer.createInfo.setSharingMode(SharingMode::eExclusive);
	uniformBuffer.Create();
	uniformBuffer.BindMemory(MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent);
}

void Renderer::CreateDescriptorPool()
{
	DescriptorPoolSize poolSize;
	poolSize.setDescriptorCount(1);
	poolSize.setType(DescriptorType::eUniformBuffer);

	DescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.setPoolSizeCount(1);
	descPoolInfo.setPPoolSizes(&poolSize);
	descPoolInfo.setMaxSets(4);

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
		descBufferInfo.setRange(sizeof(UniformBufferObject));// VK_WHOLE_SIZE;

		WriteDescriptorSet writeDescSet;
		writeDescSet.setDstSet(descriptorSets[index]);
		writeDescSet.setDstArrayElement(0);
		writeDescSet.setDescriptorCount(1);
		writeDescSet.setDescriptorType(DescriptorType::eUniformBuffer);
		writeDescSet.setPBufferInfo(&descBufferInfo);

		device.GetDevice().updateDescriptorSets(1, &writeDescSet, 0, nullptr);
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

void Renderer::UpdateUniformBuffer()
{
	ScenePtr scene = Engine::GetSceneInstance();
	CameraComponentPtr camComp = scene->GetSceneComponent<CameraComponent>();
	MeshComponentPtr meshComp = scene->GetSceneComponent<MeshComponent>();

	UniformBufferObject ubo;
	ubo.model = meshComp->GetParent()->transform.GetMatrix();
	ubo.view = camComp->CalculateViewMatrix();
	ubo.proj = camComp->CalculateProjectionMatrix(); 

	MemoryRecord& memRec = uniformBuffer.GetMemoryRecord();
	memRec.pos.memory.MapCopyUnmap(MemoryMapFlags(), memRec.pos.offset, sizeof(UniformBufferObject), &ubo, 0, sizeof(UniformBufferObject));
}

