#pragma once

#include "vulkan/vulkan.hpp"
#include "VulkanDevice.h"
#include "data/Texture2D.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::Image;
	using VULKAN_HPP_NAMESPACE::ImageView;
	using VULKAN_HPP_NAMESPACE::Semaphore;
	using VULKAN_HPP_NAMESPACE::Fence;
	using VULKAN_HPP_NAMESPACE::Framebuffer;
	using VULKAN_HPP_NAMESPACE::SwapchainKHR;
	using VULKAN_HPP_NAMESPACE::RenderPass;
	using VULKAN_HPP_NAMESPACE::Extent2D;
	using VULKAN_HPP_NAMESPACE::Format;

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
	
		std::vector<Texture2DPtr>& GetTextures() { return m_textures; }
		std::vector<Image>& GetImages() { return m_images; }
		std::vector<ImageView>& GetImageViews() { return m_imageViews; }
		Image& GetImage() { return m_images[imageIndex]; }
		Image& GetPrevImage() { return m_images[prevImageIndex]; }
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
	
		std::vector<vk::Image> m_images;
		std::vector<ImageView> m_imageViews;
		std::vector<Texture2DPtr> m_textures;
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
}
