#pragma once

#include "vulkan/vulkan.hpp"
#include "VulkanDevice.h"

using namespace VULKAN_HPP_NAMESPACE;

class VulkanSwapChain
{
public:
	VulkanSwapChain();
	virtual ~VulkanSwapChain();

	void Create(VulkanDevice* inDevice, uint32_t inBackBuffersCount);
	void Destroy();

	void CreateForResolution(uint32_t inWidth, uint32_t inHeight);
	void DestroyForResolution();

	uint32_t AcquireNextImage(bool& outBecameOutdated);
	bool Present();

	void WaitForPresentQueue();

	uint32_t GetImageIndex() { return imageIndex; }
	uint32_t GetPrevImageIndex() { return prevImageIndex; }
	Semaphore& GetImageAvailableSemaphore() { return imageAvailableSemaphores[prevImageIndex]; }
	Semaphore& GetRenderingFinishedSemaphore() { return renderingFinishedSemaphores[imageIndex]; }
	Fence& GetGraphicsQueueFence();
	Fence& GetGraphicsQueuePrevFence();
	Framebuffer& GetFramebuffer() { return framebuffers[imageIndex]; }
	Framebuffer& GetFramebuffer(uint32_t inIndex) { return framebuffers[inIndex]; }
	uint32_t GetFramebuffersCount() { return static_cast<uint32_t>(framebuffers.size()); }

	SwapchainKHR& GetSwapChain() { return swapChain; }
	RenderPass& GetRenderPass() { return renderPass; }
	Queue& GetPresentQueue() { return presentQueue; }
	Extent2D& GetExtent() { return extent; }
	Format& GetImageFormat() { return imageFormat; }

	operator SwapchainKHR() { return swapChain; }
	operator bool() { return swapChain; }
	operator Queue() { return presentQueue; }
	operator Extent2D() { return extent; }
private:
	VulkanDevice* vulkanDevice;
	SwapchainKHR swapChain;
	RenderPass renderPass;

	std::vector<Image> images;
	std::vector<ImageView> imageViews;
	std::vector<Framebuffer> framebuffers;

	std::vector<Semaphore> renderingFinishedSemaphores;
	std::vector<Semaphore> imageAvailableSemaphores;
	std::vector<Fence> cmdBuffersFences;

	SwapChainSupportDetails swapChainSupportDetails;
	SurfaceFormatKHR surfaceFormat;
	PresentModeKHR presentMode;
	Format imageFormat;
	Extent2D extent;

	Queue presentQueue;

	uint32_t imageIndex = 0;
	uint32_t prevImageIndex = 0;
	uint32_t backBuffersCount = 1;

	void CreateRenderPass();
	void DestroyRenderPass();
	void CreateRTV();
	void DestroyRTV();
	void CreateFramebuffers();
	void DestroyFramebuffers();

	SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<SurfaceFormatKHR>& inFormats);
	PresentModeKHR ChooseSwapChainPresentMode(const std::vector<PresentModeKHR>& inPresentModes);
	Extent2D ChooseSwapChainExtent(const SurfaceCapabilitiesKHR& inCapabilities, uint32_t inWidth, uint32_t inHeight);
};
