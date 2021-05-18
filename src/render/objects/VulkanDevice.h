#pragma once

#include <vulkan/vulkan.hpp>
#include "VulkanPhysicalDevice.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::Instance;
	using VULKAN_HPP_NAMESPACE::Device;
	using VULKAN_HPP_NAMESPACE::SurfaceKHR;
	using VULKAN_HPP_NAMESPACE::Queue;
	using VULKAN_HPP_NAMESPACE::PipelineCache;

	class VulkanDevice
	{
	public:
		VulkanDevice();
		virtual ~VulkanDevice();
	
		void Create(const char* inAppName, const char* inEngine, bool inValidationEnabled, HWND inHwnd);
		void Destroy();
	
		Instance& GetInstance() { return instance; }
		VulkanPhysicalDevice& GetPhysicalDevice() { return physicalDevice; }
		Device& GetDevice() { return device; }
		SurfaceKHR& GetSurface() { return surface; };
		PipelineCache& GetPipelineCache() { return pipelineCache; };
	
		Queue& GetGraphicsQueue() { return graphicsQueue; }
		Queue& GetComputeQueue() { return computeQueue; }
		Queue& GetPresentQueue() { return presentQueue; }
		Queue& GetTransferQueue() { return transferQueue; }
		uint32_t GetGraphicsQueueIndex() { return queueFamilyIndices.graphicsFamily.value(); }
		uint32_t GetComputeQueueIndex() { return queueFamilyIndices.computeFamily.value(); }
		uint32_t GetPresentQueueIndex() { return queueFamilyIndices.presentFamily.value(); }
		uint32_t GetTransferQueueIndex() { return queueFamilyIndices.transferFamily.value(); }
	
		operator Instance() { return instance; }
		operator Device() { return device; }
		operator PhysicalDevice() { return physicalDevice.GetDevice(); }
		operator SurfaceKHR() { return surface; }
		operator PipelineCache() { return pipelineCache; }
	protected:
		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		std::vector<const char*> requiredExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	
		Instance instance;
		VulkanPhysicalDevice physicalDevice;
		Device device;
		SurfaceKHR surface;
	
		QueueFamilyIndices queueFamilyIndices;
	
		Queue graphicsQueue;
		Queue computeQueue;
		Queue presentQueue;
		Queue transferQueue;
	
		PipelineCache pipelineCache;
	
		bool CheckValidationLayerSupport();
		VulkanPhysicalDevice PickPhysicalDevice(std::vector<PhysicalDevice>& inDevices);
		int ScoreDeviceSuitability(VulkanPhysicalDevice& inPhysicalDevice);
		int IsDeviceUsable(VulkanPhysicalDevice& inPhysicalDevice, SurfaceKHR& inSurface);
	};
	
	
	
}
