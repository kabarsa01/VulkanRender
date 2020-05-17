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
#include "PerFrameData.h"
#include "passes/GBufferPass.h"
#include "passes/ZPrepass.h"
#include "passes/PostProcessPass.h"

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

void Renderer::Init()
{
	GLFWwindow* window = Engine::GetInstance()->GetGlfwWindow();
	glfwGetFramebufferSize(window, &width, &height);

	HWND hWnd = glfwGetWin32Window(Engine::GetInstance()->GetGlfwWindow());
	device.Create("VulkanRenderer", "VulkanEngine", enableValidationLayers, hWnd);
	swapChain.Create(&device, 2);
	swapChain.CreateForResolution(width, height);
	commandBuffers.Create(&device, 2, 1);
	descriptorPools.Create(&device);

	perFrameData = new PerFrameData();
	perFrameData->Create(&device);

	zPrepass = new ZPrepass(HashString("ZPrepass"));
	zPrepass->Create();
	gBufferPass = new GBufferPass(HashString("GBufferPass"));
	gBufferPass->SetExternalDepth(zPrepass->GetDepthAttachment(), zPrepass->GetDepthAttachmentView());
	gBufferPass->Create();
	postProcessPass = new PostProcessPass(HashString("PostProcessPass"));
	postProcessPass->Create();
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
	// z prepass
	zPrepass->Draw(&cmdBuffer);
	// gbuffer pass
	gBufferPass->Draw(&cmdBuffer);
	// barriers ----------------------------------------------
	const std::vector<VulkanImage>& gBufferAttachments = gBufferPass->GetAttachments();
	std::vector<ImageMemoryBarrier> gBufferBarriers;
	for (uint32_t index = 0; index < gBufferAttachments.size(); index++)
	{
		ImageMemoryBarrier attachmentBarrier = gBufferAttachments[index].CreateLayoutBarrier(
			ImageLayout::eColorAttachmentOptimal,
			ImageLayout::eShaderReadOnlyOptimal,
			AccessFlagBits::eColorAttachmentWrite,
			AccessFlagBits::eShaderRead,
			ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		gBufferBarriers.push_back(attachmentBarrier);
	}
	cmdBuffer.pipelineBarrier(
		PipelineStageFlagBits::eColorAttachmentOutput,
		PipelineStageFlagBits::eFragmentShader,
		DependencyFlags(),
		0, nullptr, 0, nullptr,
		static_cast<uint32_t>(gBufferBarriers.size()),
		gBufferBarriers.data());
	//--------------------------------------------------------
	// post process pass
	postProcessPass->Draw(&cmdBuffer);
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
	device.GetGraphicsQueue().waitIdle();
	//swapChain.WaitForPresentQueue();
}

void Renderer::WaitForDevice()
{
	device.GetDevice().waitIdle();
}

void Renderer::Cleanup()
{
	WaitForDevice();

	postProcessPass->Destroy();
	delete postProcessPass;
	gBufferPass->Destroy();
	delete gBufferPass;
	zPrepass->Destroy();
	delete zPrepass;

	ScenePtr scene = Engine::GetSceneInstance();
	MeshComponentPtr meshComp = scene->GetSceneComponent<MeshComponent>();
	meshComp->meshData->DestroyBuffer();

	//uniformBuffer.Destroy();

	perFrameData->Destroy();
	delete perFrameData;
	PipelineRegistry::GetInstance()->DestroyPipelines(&device);

	descriptorPools.Destroy();

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

VulkanDescriptorPools& Renderer::GetDescriptorPools()
{
	return descriptorPools;
}

Queue Renderer::GetGraphicsQueue()
{
	return device.GetGraphicsQueue();
}

void Renderer::OnResolutionChange()
{
	device.GetDevice().waitIdle();

	GLFWwindow* window = Engine::GetInstance()->GetGlfwWindow();
	glfwGetFramebufferSize(window, &width, &height);

	postProcessPass->Destroy();
	delete postProcessPass;
	swapChain.DestroyForResolution();

	swapChain.CreateForResolution(width, height);
	postProcessPass = new PostProcessPass("PostProcessPass");
	postProcessPass->Create();
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

