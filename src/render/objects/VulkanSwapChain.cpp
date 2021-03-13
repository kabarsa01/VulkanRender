#include "VulkanSwapChain.h"
#include "core/Engine.h"
#include <algorithm>

namespace CGE
{
	namespace vk = VULKAN_HPP_NAMESPACE;

	VulkanSwapChain::VulkanSwapChain()
	{
	
	}
	
	VulkanSwapChain::~VulkanSwapChain()
	{
	
	}
	
	void VulkanSwapChain::Create(VulkanDevice* inDevice, uint32_t inBackBuffersCount)
	{
		vulkanDevice = inDevice;
		backBuffersCount = inBackBuffersCount;
	
		Device& device = vulkanDevice->GetDevice();
		presentQueue = vulkanDevice->GetPresentQueue();
	
		// create fences and semaphores
		imageAvailableSemaphores.resize(backBuffersCount);
		renderingFinishedSemaphores.resize(backBuffersCount);
		cmdBuffersFences.resize(backBuffersCount);
		for (uint32_t index = 0; index < backBuffersCount; index++)
		{
			vk::FenceCreateInfo fenceInfo;
			fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
			cmdBuffersFences[index] = device.createFence(fenceInfo);
	
			imageAvailableSemaphores[index] = device.createSemaphore(vk::SemaphoreCreateInfo());
			renderingFinishedSemaphores[index] = device.createSemaphore(vk::SemaphoreCreateInfo());
		}
	}
	
	void VulkanSwapChain::Destroy()
	{
		Device& device = vulkanDevice->GetDevice();
		device.waitForFences(backBuffersCount, cmdBuffersFences.data(), VK_TRUE, UINT64_MAX);
	
		DestroyForResolution();
	
		for (uint32_t index = 0; index < imageAvailableSemaphores.size(); index++)
		{
			device.destroySemaphore(imageAvailableSemaphores[index]);
			device.destroySemaphore(renderingFinishedSemaphores[index]);
			device.destroyFence(cmdBuffersFences[index]);
		}
		imageAvailableSemaphores.clear();
		renderingFinishedSemaphores.clear();
		cmdBuffersFences.clear();
	}
	
	
	void VulkanSwapChain::CreateForResolution(uint32_t inWidth, uint32_t inHeight)
	{
		Device& device = vulkanDevice->GetDevice();
	
		swapChainSupportDetails = vulkanDevice->GetPhysicalDevice().QuerySwapChainSupport(*vulkanDevice);
		surfaceFormat = ChooseSurfaceFormat(swapChainSupportDetails.formats);
		presentMode = ChooseSwapChainPresentMode(swapChainSupportDetails.presentModes);
		imageFormat = surfaceFormat.format;
	
		DestroyRenderPass();
		CreateRenderPass();
	
		//int windowWidth, windowHeight;
		//GLFWwindow* window = Engine::GetInstance()->GetGlfwWindow();
		//glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
		extent = ChooseSwapChainExtent(swapChainSupportDetails.capabilities, inWidth, inHeight);
		SurfaceCapabilitiesKHR& capabilities = swapChainSupportDetails.capabilities;
		//uint32_t imageCount = capabilities.minImageCount + 1;
		//if (capabilities.maxImageCount > 0 && (imageCount > capabilities.maxImageCount))
		//{
		//	imageCount = capabilities.maxImageCount;
		//}
	
		vk::SwapchainCreateInfoKHR createInfo;
		createInfo.setSurface(*vulkanDevice);
		createInfo.setMinImageCount(backBuffersCount);
		createInfo.setImageFormat(surfaceFormat.format);
		createInfo.setImageColorSpace(surfaceFormat.colorSpace);
		createInfo.setImageExtent(extent);
		createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);// ImageUsageFlagBits::eTransferDst or ImageUsageFlagBits::eDepthStencilAttachment
		createInfo.setImageArrayLayers(1);
		createInfo.setPreTransform(capabilities.currentTransform);
		createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
		createInfo.setPresentMode(presentMode);
		createInfo.setClipped(VK_TRUE);
		createInfo.setOldSwapchain(SwapchainKHR());
	
		QueueFamilyIndices queueFamilyIndices = vulkanDevice->GetPhysicalDevice().GetCachedQueueFamiliesIndices();
		if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
		{
			uint32_t familyIndices[] = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };
			createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
			createInfo.setQueueFamilyIndexCount(2);
			createInfo.setPQueueFamilyIndices(familyIndices);
		}
		else
		{
			uint32_t familyIndices[] = { queueFamilyIndices.graphicsFamily.value() };
			createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
			createInfo.setQueueFamilyIndexCount(1);
			createInfo.setPQueueFamilyIndices(familyIndices);
		}
	
		swapChain = device.createSwapchainKHR(createInfo);
		images = device.getSwapchainImagesKHR(swapChain);
	
		CreateRTV();
		CreateFramebuffers();
	
		imageIndex = 0;
		prevImageIndex = backBuffersCount - 1;
	}
	
	void VulkanSwapChain::DestroyForResolution()
	{
		DestroyRenderPass();
		DestroyFramebuffers();
		DestroyRTV();
	
		if (swapChain)
		{
			vulkanDevice->GetDevice().destroySwapchainKHR(swapChain);
		}
	}
	
	uint32_t VulkanSwapChain::AcquireNextImage(bool& outBecameOutdated)
	{
		outBecameOutdated = false;
		prevImageIndex = imageIndex;
	
		Device& device = vulkanDevice->GetDevice();
	
		vk::ResultValue<uint32_t> imageIndexResult = vk::ResultValue<uint32_t>(vk::Result::eSuccess, imageIndex);
	
		try
		{
			imageIndexResult = device.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[imageIndex], Fence());
		}
		catch (std::exception exc)
		{
		}
	
		if (imageIndexResult.result == vk::Result::eErrorOutOfDateKHR || imageIndexResult.result == vk::Result::eErrorIncompatibleDisplayKHR)
		{
			outBecameOutdated = true;
	//		return 0;
		}
		else if (imageIndexResult.result != vk::Result::eSuccess && imageIndexResult.result != vk::Result::eSuboptimalKHR)
		{
			throw std::runtime_error("failed to acquire swap chain image");
		}
		imageIndex = imageIndexResult.value;
	
		vulkanDevice->GetDevice().waitForFences(1, &cmdBuffersFences[imageIndex], VK_TRUE, UINT64_MAX);
		vulkanDevice->GetDevice().resetFences(1, &cmdBuffersFences[imageIndex]);
	
		return imageIndex;
	}
	
	bool VulkanSwapChain::Present()
	{
		vk::SwapchainKHR swapChains[] = { swapChain };
		vk::PresentInfoKHR presentInfo;
		presentInfo.setWaitSemaphoreCount(1);
		presentInfo.setPWaitSemaphores(&renderingFinishedSemaphores[imageIndex]);
		presentInfo.setSwapchainCount(1);
		presentInfo.setPSwapchains(swapChains);
		presentInfo.setPImageIndices(&imageIndex);
		presentInfo.setPResults(nullptr);
	
		vk::Result presentResult = vk::Result::eSuccess;
		try
		{
			presentResult = presentQueue.presentKHR(presentInfo);
		}
		catch (std::exception)
		{
		}
	
		if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
		{
			return false;
		}
	
		return true;
	}
	
	void VulkanSwapChain::WaitForPresentQueue()
	{
		presentQueue.waitIdle();
	}
	
	Fence& VulkanSwapChain::GetGraphicsQueueFence()
	{
		return cmdBuffersFences[imageIndex];
	}
	
	Fence& VulkanSwapChain::GetGraphicsQueuePrevFence()
	{
		return cmdBuffersFences[prevImageIndex];
	}
	
	void VulkanSwapChain::CreateRenderPass()
	{
		vk::AttachmentDescription colorAttachment;
		colorAttachment.setFormat(imageFormat);
		colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
		colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
		colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
		colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
		colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
		colorAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
	
		vk::AttachmentReference colorAttachmentRef;
		colorAttachmentRef.setAttachment(0);
		colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	
		vk::SubpassDescription subpassDesc;
		subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
		subpassDesc.setColorAttachmentCount(1);
		subpassDesc.setPColorAttachments(&colorAttachmentRef); // layout(location = |index| ) out vec4 outColor references attachmant by index
	
		vk::SubpassDependency subpassDependency;
		subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
		subpassDependency.setDstSubpass(0);
		subpassDependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		subpassDependency.setSrcAccessMask(vk::AccessFlags());
		subpassDependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		subpassDependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
	
		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo.setAttachmentCount(1);
		renderPassInfo.setPAttachments(&colorAttachment);
		renderPassInfo.setSubpassCount(1);
		renderPassInfo.setPSubpasses(&subpassDesc);
		renderPassInfo.setDependencyCount(1);
		renderPassInfo.setPDependencies(&subpassDependency);
	
		renderPass = vulkanDevice->GetDevice().createRenderPass(renderPassInfo);
	}
	
	void VulkanSwapChain::DestroyRenderPass()
	{
		if (renderPass && vulkanDevice)
		{
			vulkanDevice->GetDevice().destroyRenderPass(renderPass);
			renderPass = nullptr;
		}
	}
	
	void VulkanSwapChain::CreateRTV()
	{
		imageViews.resize(images.size());
		for (uint32_t index = 0; index < images.size(); index++)
		{
			vk::ImageViewCreateInfo createInfo;
			createInfo.setImage(images[index]);
			createInfo.setViewType(vk::ImageViewType::e2D);
			createInfo.setFormat(imageFormat);
			createInfo.setComponents(vk::ComponentMapping());
			createInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
	
			imageViews[index] = vulkanDevice->GetDevice().createImageView(createInfo);
		}
	}
	
	void VulkanSwapChain::DestroyRTV()
	{
		for (uint32_t index = 0; index < imageViews.size(); index++)
		{
			vulkanDevice->GetDevice().destroyImageView(imageViews[index]);
		}
		imageViews.clear();
	}
	
	void VulkanSwapChain::CreateFramebuffers()
	{
		framebuffers.resize(backBuffersCount);
		for (int index = 0; index < imageViews.size(); index++)
		{
			ImageView attachments[] = { imageViews[index] };
	
			vk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.setRenderPass(renderPass);
			framebufferInfo.setAttachmentCount(1);
			framebufferInfo.setPAttachments(attachments);
			framebufferInfo.setWidth(extent.width);
			framebufferInfo.setHeight(extent.height);
			framebufferInfo.setLayers(1);
	
			framebuffers[index] = vulkanDevice->GetDevice().createFramebuffer(framebufferInfo);
		}
	}
	
	void VulkanSwapChain::DestroyFramebuffers()
	{
		for (uint32_t index = 0; index < framebuffers.size(); index++)
		{
			vulkanDevice->GetDevice().destroyFramebuffer(framebuffers[index]);
		}
		framebuffers.clear();
	}
	
	SurfaceFormatKHR VulkanSwapChain::ChooseSurfaceFormat(const std::vector<SurfaceFormatKHR>& inFormats)
	{
		for (const SurfaceFormatKHR& surfaceFormat : inFormats)
		{
			if (surfaceFormat.format == Format::eR8G8B8A8Unorm && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return surfaceFormat;
			}
		}
	
		return inFormats[0];
	}
	
	PresentModeKHR VulkanSwapChain::ChooseSwapChainPresentMode(const std::vector<PresentModeKHR>& inPresentModes)
	{
		for (const PresentModeKHR& presentMode : inPresentModes)
		{
			if (presentMode == PresentModeKHR::eMailbox)
			{
				return presentMode;
			}
		}
	
		return PresentModeKHR::eFifo;
	}
	
	Extent2D VulkanSwapChain::ChooseSwapChainExtent(const SurfaceCapabilitiesKHR& inCapabilities, uint32_t inWidth, uint32_t inHeight)
	{
		if (inCapabilities.currentExtent.width == UINT32_MAX)
		{
			return inCapabilities.currentExtent;
		}
		else
		{
			Extent2D extent;
	
			extent.setWidth(std::clamp<int>(inWidth, inCapabilities.minImageExtent.width, inCapabilities.maxImageExtent.width));
			extent.setHeight(std::clamp<int>(inHeight, inCapabilities.minImageExtent.height, inCapabilities.maxImageExtent.height));
	
			return extent;
		}
	}
	
	
}
