#pragma once

#include "vulkan/vulkan.hpp"
#include "VulkanPhysicalDevice.h"

using namespace VULKAN_HPP_NAMESPACE;

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

	Queue& GetGraphicsQueue() { return graphicsQueue; }
	Queue& GetComputeQueue() { return computeQueue; }
	Queue& GetPresentQueue() { return presentQueue; }

	operator Instance() { return instance; }
	operator Device() { return device; }
	operator PhysicalDevice() { return physicalDevice.GetDevice(); }
	operator SurfaceKHR() { return surface; }
protected:
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
	std::vector<const char*> requiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
//#ifdef NDEBUG
//	const bool enableValidationLayers = false;
//#else
//	const bool enableValidationLayers = true;
//#endif

	Instance instance;
	VulkanPhysicalDevice physicalDevice;
	Device device;
	SurfaceKHR surface;

	QueueFamilyIndices queueFamilyIndices;

	Queue graphicsQueue;
	Queue computeQueue;
	Queue presentQueue;

	bool CheckValidationLayerSupport();
	VulkanPhysicalDevice PickPhysicalDevice(std::vector<PhysicalDevice>& inDevices);
	int ScoreDeviceSuitability(VulkanPhysicalDevice& inPhysicalDevice);
	int IsDeviceUsable(VulkanPhysicalDevice& inPhysicalDevice, SurfaceKHR& inSurface);
};



