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
#include "PipelineRegistry.h"
#include "passes/DeferredLightingPass.h"
#include "passes/LightClusteringPass.h"
#include "utils/Singleton.h"
#include "RtScene.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::CommandBufferBeginInfo;
	using VULKAN_HPP_NAMESPACE::CommandBufferUsageFlagBits;
	using VULKAN_HPP_NAMESPACE::AccessFlagBits;
	using VULKAN_HPP_NAMESPACE::ImageAspectFlagBits;
	using VULKAN_HPP_NAMESPACE::PipelineStageFlagBits;
	using VULKAN_HPP_NAMESPACE::DependencyFlags;
	using VULKAN_HPP_NAMESPACE::SubmitInfo;
	using VULKAN_HPP_NAMESPACE::PipelineStageFlags;
	using VULKAN_HPP_NAMESPACE::ArrayProxy;
	using VULKAN_HPP_NAMESPACE::ImageBlit;
	using VULKAN_HPP_NAMESPACE::Offset3D;
	using VULKAN_HPP_NAMESPACE::ImageSubresourceLayers;
	using VULKAN_HPP_NAMESPACE::Filter;

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
		zPrepass->SetResolution(width, height);
		zPrepass->Create();
		lightClusteringPass = new LightClusteringPass(HashString("LightClusteringPass"));
		lightClusteringPass->Create();
		gBufferPass = new GBufferPass(HashString("GBufferPass"));
		gBufferPass->SetExternalDepth(zPrepass->GetDepthAttachment(), zPrepass->GetDepthAttachmentView());
		gBufferPass->SetResolution(width, height);
		gBufferPass->Create();
		deferredLightingPass = new DeferredLightingPass(HashString("DeferredLightingPass"));
		deferredLightingPass->SetResolution(width, height);
		deferredLightingPass->Create();
		postProcessPass = new PostProcessPass(HashString("PostProcessPass"));
		postProcessPass->SetResolution(width, height);
		postProcessPass->Create();

		Singleton<RtScene>::GetInstance()->Init();
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
		lightClusteringPass->UpdateData();
	
		CommandBuffer& cmdBuffer = commandBuffers.GetNextForPool(imageIndex);
	
		CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(CommandBufferUsageFlagBits::eSimultaneousUse);
		beginInfo.setPInheritanceInfo(nullptr);
	
		// begin command buffer record
		cmdBuffer.begin(beginInfo);

		Singleton<RtScene>::GetInstance()->UpdateShaders();
		Singleton<RtScene>::GetInstance()->BuildMeshBlases(&cmdBuffer);
		Singleton<RtScene>::GetInstance()->UpdateInstances();
		// copy new data
		TransferResources(cmdBuffer, device.GetPhysicalDevice().GetCachedQueueFamiliesIndices().graphicsFamily.value());

		Singleton<RtScene>::GetInstance()->BuildSceneTlas(&cmdBuffer);

		// render passes
		// z prepass
		zPrepass->RecordCommands(&cmdBuffer);
		//----------------------------------------------------------
		// light clustering pass
		lightClusteringPass->RecordCommands(&cmdBuffer);
		//--------------------------------------------------------
		// gbuffer pass
		gBufferPass->RecordCommands(&cmdBuffer);
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
		// deferred lighting pass
		deferredLightingPass->RecordCommands(&cmdBuffer);
		//--------------------------------------------------------
		ImageMemoryBarrier attachmentBarrier = deferredLightingPass->GetAttachments()[0].CreateLayoutBarrier(
			ImageLayout::eColorAttachmentOptimal,
			ImageLayout::eShaderReadOnlyOptimal,
			AccessFlagBits::eColorAttachmentWrite,
			AccessFlagBits::eShaderRead,
			ImageAspectFlagBits::eColor,
			0, 1, 0, 1);
		cmdBuffer.pipelineBarrier(
			PipelineStageFlagBits::eColorAttachmentOutput,
			PipelineStageFlagBits::eFragmentShader,
			DependencyFlags(),
			0, nullptr, 0, nullptr,
			1, &attachmentBarrier);
		//--------------------------------------------------------
		// post process pass
		postProcessPass->RecordCommands(&cmdBuffer);
		// end commands recording
		cmdBuffer.end();
	
		SubmitInfo submitInfo{};
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
		deferredLightingPass->Destroy();
		delete deferredLightingPass;
		gBufferPass->Destroy();
		delete gBufferPass;
		zPrepass->Destroy();
		delete zPrepass;
		lightClusteringPass->Destroy();
		delete lightClusteringPass;
	
		Scene* scene = Engine::GetSceneInstance();
		MeshComponentPtr meshComp = scene->GetSceneComponent<MeshComponent>();
		meshComp->meshData->DestroyBuffer();
	
		perFrameData->Destroy();
		delete perFrameData;
	
		PipelineRegistry::GetInstance()->DestroyPipelines(&device);

		Singleton<RtScene>::GetInstance()->Cleanup();
	
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
		postProcessPass->SetResolution(width, height);
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
				0, images[index]->GetMips(), 0, 1);
			afterTransferBarriers[index] = images[index]->CreateLayoutBarrier(
				ImageLayout::eUndefined,
				ImageLayout::eShaderReadOnlyOptimal,
				AccessFlagBits::eTransferWrite,
				AccessFlagBits::eShaderRead,
				ImageAspectFlagBits::eColor,
				0, images[index]->GetMips(), 0, 1);
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
	
		GenerateMips(inCmdBuffer, images);
	
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
	
	void Renderer::GenerateMips(CommandBuffer& inCmdBuffer, std::vector<VulkanImage*>& inImages)
	{
		for (uint32_t index = 0; index < inImages.size(); index++)
		{
			VulkanImage* image = inImages[index];
	
			for (uint32_t mipIndex = 1; mipIndex < image->GetMips(); mipIndex++)
			{
				uint32_t previousWidth = std::max(image->GetWidth() >> (mipIndex - 1), (uint32_t)1);
				uint32_t previousHeight = std::max(image->GetHeight() >> (mipIndex - 1), (uint32_t)1);
				uint32_t currentWidth = std::max(previousWidth >> 1, (uint32_t)1);
				uint32_t currentHeight = std::max(previousHeight >> 1, (uint32_t)1);
	
				ImageBlit blit;
				std::array<Offset3D, 2> srcOffsets = { Offset3D(0, 0, 0), Offset3D(previousWidth, previousHeight, 1) };
				blit.setSrcOffsets(srcOffsets);
				blit.setSrcSubresource(ImageSubresourceLayers(ImageAspectFlagBits::eColor, mipIndex - 1, 0, 1));
				std::array<Offset3D, 2> dstOffsets = { Offset3D(0, 0, 0), Offset3D(currentWidth, currentHeight, 1) };
				blit.setDstOffsets(dstOffsets);
				blit.setDstSubresource(ImageSubresourceLayers(ImageAspectFlagBits::eColor, mipIndex, 0, 1));
	
				// change layout for source and destination mip to prepare for copy
				ImageMemoryBarrier previousMipBarrier = image->CreateLayoutBarrier(
					ImageLayout::eUndefined,
					ImageLayout::eTransferSrcOptimal,
					AccessFlagBits::eTransferWrite,
					AccessFlagBits::eTransferRead,
					ImageAspectFlagBits::eColor,
					mipIndex - 1, 1, 0, 1);
				ImageMemoryBarrier currentMipBarrier = image->CreateLayoutBarrier(
					ImageLayout::eUndefined,
					ImageLayout::eTransferDstOptimal,
					AccessFlagBits::eTransferRead,
					AccessFlagBits::eTransferWrite,
					ImageAspectFlagBits::eColor,
					mipIndex, 1, 0, 1);
	
				ImageMemoryBarrier barriers[] = { previousMipBarrier, currentMipBarrier };
				inCmdBuffer.pipelineBarrier(
					PipelineStageFlagBits::eTransfer,
					PipelineStageFlagBits::eTransfer,
					DependencyFlags(),
					0, nullptr,
					0, nullptr,
					2, barriers);
	
				inCmdBuffer.blitImage(*image, ImageLayout::eTransferSrcOptimal, *image, ImageLayout::eTransferDstOptimal, { blit }, Filter::eLinear);
			}
		}
	}
	
}
