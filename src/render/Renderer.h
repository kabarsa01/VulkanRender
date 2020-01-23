#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <optional>
#include <core/ObjectBase.h>
#include "fwd.hpp"
#include <set>

//-------------------------------------------------------------------------------------------------------

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> computeFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete()
	{
		return graphicsFamily.has_value() && computeFamily.has_value() && presentFamily.has_value();
	}

	std::set<uint32_t> GetFamiliesSet()
	{
		return std::set<uint32_t>{
			graphicsFamily.value(),
			computeFamily.value(),
			presentFamily.value()
		};
	}
};

//-------------------------------------------------------------------------------------------------------

struct SwapChainSupportDetails 
{
	VULKAN_HPP_NAMESPACE::SurfaceCapabilitiesKHR capabilities;
	std::vector<VULKAN_HPP_NAMESPACE::SurfaceFormatKHR> formats;
	std::vector<VULKAN_HPP_NAMESPACE::PresentModeKHR> presentModes;

	bool IsUsable()
	{
		return !formats.empty() && !presentModes.empty();
	}
};

//=======================================================================================================
//=======================================================================================================

class Renderer : public ObjectBase
{
public:
	Renderer();
	virtual ~Renderer();

	virtual void OnInitialize() override;

	void Init();
	void RenderFrame();
	void Cleanup();

	void SetResolution(int InWidth, int InHeight);
	int GetWidth() const;
	int GetHeight() const;
protected:
private:
	//======================= VARS ===============================
	uint32_t version;
	int width = 1280;
	int height = 720;

	VULKAN_HPP_NAMESPACE::Instance vulkanInstance;
	VULKAN_HPP_NAMESPACE::SurfaceKHR vulkanSurface;
	VULKAN_HPP_NAMESPACE::PhysicalDevice vulkanPhysicalDevice;
	VULKAN_HPP_NAMESPACE::Device vulkanDevice;
	VULKAN_HPP_NAMESPACE::SwapchainKHR vulkanSwapChain;
	std::vector<VULKAN_HPP_NAMESPACE::Image> swapChainImages;
	std::vector<VULKAN_HPP_NAMESPACE::ImageView> swapChainImageViews;
	VULKAN_HPP_NAMESPACE::Format swapChainImageFormat;
	VULKAN_HPP_NAMESPACE::Extent2D swapChainExtent;
	VULKAN_HPP_NAMESPACE::Queue graphicsQueue;
	VULKAN_HPP_NAMESPACE::Queue computeQueue;
	VULKAN_HPP_NAMESPACE::Queue presentQueue;

	std::vector<const char*> requiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	//std::vector<RenderPassPtr> RenderPasses;
	//std::map<HashString, unsigned int> RenderPassMap;
	//==================== METHODS ===============================

	void PickPhysicalDevice(std::vector<VULKAN_HPP_NAMESPACE::PhysicalDevice>& inDevices);
	int ScoreDeviceSuitability(const VULKAN_HPP_NAMESPACE::PhysicalDevice& inPhysicalDevice);

	int IsDeviceUsable(const VULKAN_HPP_NAMESPACE::PhysicalDevice& inPhysicalDevice);

	bool CheckPhysicalDeviceExtensionSupport(const VULKAN_HPP_NAMESPACE::PhysicalDevice& inPhysicalDevice);
	QueueFamilyIndices FindQueueFamilies(const VULKAN_HPP_NAMESPACE::PhysicalDevice& inPhysicalDevice);
	SwapChainSupportDetails QuerySwapChainSupport(const VULKAN_HPP_NAMESPACE::PhysicalDevice& inPhysicalDevice);
	void CreateLogicalDevice();

	VULKAN_HPP_NAMESPACE::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VULKAN_HPP_NAMESPACE::SurfaceFormatKHR>& inFormats);
	VULKAN_HPP_NAMESPACE::PresentModeKHR ChooseSwapChainPresentMode(const std::vector<VULKAN_HPP_NAMESPACE::PresentModeKHR>& inPresentModes);
	VULKAN_HPP_NAMESPACE::Extent2D ChooseSwapChainExtent(const VULKAN_HPP_NAMESPACE::SurfaceCapabilitiesKHR& inCapabilities);
	void CreateSwapChain();
	void CreateImageViews();

	void CreateGraphicsPipeline();
//	void RegisterRenderPass(RenderPassPtr InRenderPass);
};

typedef std::shared_ptr<Renderer> RendererPtr;

