#include "VulkanDevice.h"
#include "core/Engine.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include "../memory/DeviceMemoryManager.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::ApplicationInfo;
	using VULKAN_HPP_NAMESPACE::InstanceCreateInfo;
	using VULKAN_HPP_NAMESPACE::Win32SurfaceCreateInfoKHR;
	using VULKAN_HPP_NAMESPACE::DeviceQueueCreateInfo;
	using VULKAN_HPP_NAMESPACE::DeviceCreateInfo;
	using VULKAN_HPP_NAMESPACE::PipelineCacheCreateInfo;
	using VULKAN_HPP_NAMESPACE::LayerProperties;

	VulkanDevice::VulkanDevice()
	{
	
	}
	
	VulkanDevice::~VulkanDevice()
	{
	
	}
	
	void VulkanDevice::Create(const char* inAppName, const char* inEngine, bool inValidationEnabled, HWND inHwnd)
	{
		vk::DynamicLoader dl;
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		if (inValidationEnabled && !CheckValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}
	
		ApplicationInfo applicationInfo(
			inAppName,
			1,
			inEngine,
			1,
			VK_API_VERSION_1_2
		);
	
		uint32_t glfwInstanceExtensionsCount;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionsCount);
		std::vector<const char*> instanceExtensions;
		for (uint32_t idx = 0; idx < glfwInstanceExtensionsCount; idx++)
		{
			instanceExtensions.push_back(glfwExtensions[idx]);
		}
		instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		for (auto ext : requiredDeviceExtensions)
		{
//			instanceExtensions.push_back(ext);
		}
	
		InstanceCreateInfo instanceCreateInfo;
		instanceCreateInfo.setPApplicationInfo(&applicationInfo);
		instanceCreateInfo.setEnabledExtensionCount(static_cast<uint32_t>(instanceExtensions.size()));
		instanceCreateInfo.setPpEnabledExtensionNames(instanceExtensions.data());
	
		if (inValidationEnabled)
		{
			instanceCreateInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
			instanceCreateInfo.setPpEnabledLayerNames(validationLayers.data());
		}
		else
		{
			instanceCreateInfo.setEnabledLayerCount(0);
		}
	
		instance = createInstance(instanceCreateInfo);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

		Win32SurfaceCreateInfoKHR surfaceCreateInfo;
		surfaceCreateInfo.setHwnd(inHwnd);
		surfaceCreateInfo.setHinstance(GetModuleHandle(nullptr));
		surface = instance.createWin32SurfaceKHR(surfaceCreateInfo);
	
		//-----------------------------------------------------------------------------------------------------
	
		std::vector<ExtensionProperties> extensionsResult = VULKAN_HPP_NAMESPACE::enumerateInstanceExtensionProperties();
		for (ExtensionProperties extensionProperties : extensionsResult)
		{
			std::cout << "\t" << extensionProperties.extensionName << std::endl;
		}
		std::vector<PhysicalDevice> devices = instance.enumeratePhysicalDevices();
		physicalDevice = PickPhysicalDevice(devices);
		queueFamilyIndices = physicalDevice.GetQueueFamiliesIndices(surface, true);
		
		std::vector<DeviceQueueCreateInfo> queueCreateInfoVector;
		std::set<uint32_t> indicesSet = physicalDevice.GetQueueFamiliesIndicesSet(QueueFlagBits::eGraphics | QueueFlagBits::eCompute, surface);
		for (uint32_t queueFamiltIndex : indicesSet)
		{
			DeviceQueueCreateInfo queueCreateInfo;
			queueCreateInfo.setQueueFamilyIndex(queueFamiltIndex);
			queueCreateInfo.setQueueCount(1); // magic 1
			float priority = 0.0f;
			queueCreateInfo.setPQueuePriorities(&priority);
	
			queueCreateInfoVector.push_back(queueCreateInfo);
		}
	
		PhysicalDeviceFeatures deviceFeatures;
		deviceFeatures.setFragmentStoresAndAtomics(VK_TRUE);

		vk::PhysicalDeviceFeatures2 features2;
		//vk::PhysicalDeviceVulkan11Features features11;
		vk::PhysicalDeviceVulkan12Features features12;
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelFeature;
		vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeature;
		features2.pNext = &features12;
		features12.pNext = &accelFeature;
		accelFeature.pNext = &rtPipelineFeature;
		physicalDevice.GetDevice().getFeatures2(&features2);
	
		DeviceCreateInfo deviceCreateInfo;
		deviceCreateInfo.setPQueueCreateInfos(queueCreateInfoVector.data());
		deviceCreateInfo.setQueueCreateInfoCount((uint32_t)queueCreateInfoVector.size());
		deviceCreateInfo.setEnabledExtensionCount((uint32_t)requiredDeviceExtensions.size());
		deviceCreateInfo.setPpEnabledExtensionNames(requiredDeviceExtensions.data());
		deviceCreateInfo.setEnabledLayerCount(0);
		deviceCreateInfo.pNext = &features2;
	
		device = physicalDevice.GetDevice().createDevice(deviceCreateInfo);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
	
		graphicsQueue = device.getQueue(queueFamilyIndices.graphicsFamily.value(), 0);
		computeQueue = device.getQueue(queueFamilyIndices.computeFamily.value(), 0);
		presentQueue = device.getQueue(queueFamilyIndices.presentFamily.value(), 0);
		transferQueue = device.getQueue(queueFamilyIndices.transferFamily.value(), 0);
	
		PipelineCacheCreateInfo pipelineCacheInfo;
		// TODO configure initial size for cache of whatever
		pipelineCache = device.createPipelineCache(pipelineCacheInfo);
	}
	
	void VulkanDevice::Destroy()
	{
		device.destroyPipelineCache(pipelineCache);
	
		DeviceMemoryManager::GetInstance()->CleanupMemory();
		device.destroy();
	
		instance.destroySurfaceKHR(surface);
		instance.destroy();
	}
	
	bool VulkanDevice::CheckValidationLayerSupport()
	{
		std::vector<LayerProperties> layerProps = VULKAN_HPP_NAMESPACE::enumerateInstanceLayerProperties();
	
		for (std::string layer : validationLayers)
		{
			bool layerFound = false;
			for (LayerProperties prop : layerProps)
			{
				std::string availableLayerName(&prop.layerName[0]);
				if (availableLayerName == layer)
				{
					layerFound = true;
					break;
				}
			}
			if (!layerFound)
			{
				return false;
			}
		}
	
		return true;
	}
	
	VulkanPhysicalDevice VulkanDevice::PickPhysicalDevice(std::vector<PhysicalDevice>& inDevices)
	{
		std::vector<VulkanPhysicalDevice> devices;
		for (PhysicalDevice& device : inDevices)
		{
			devices.push_back(device);
		}
		// Use an ordered map to automatically sort candidates by increasing score
		std::multimap<int, VulkanPhysicalDevice> candidates;
	
		for (VulkanPhysicalDevice& device : devices) {
			int score = ScoreDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}
	
		// Check if the best candidate is suitable at all
		if (candidates.rbegin()->first > 0) {
			return candidates.rbegin()->second;
		}
		else {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}
	
	int VulkanDevice::ScoreDeviceSuitability(VulkanPhysicalDevice& inPhysicalDevice)
	{
		if (!IsDeviceUsable(inPhysicalDevice, surface))
		{
			return 0;
		}
		PhysicalDeviceFeatures physicalDeviceFeatures = inPhysicalDevice.GetFeatures();
		// Application can't function without geometry shaders
		if (!physicalDeviceFeatures.geometryShader) {
			return 0;
		}
	
		int score = 0;
		PhysicalDeviceProperties physicalDeviceProperties = inPhysicalDevice.GetProperties();
		// Discrete GPUs have a significant performance advantage
		if (physicalDeviceProperties.deviceType == PhysicalDeviceType::eDiscreteGpu) {
			score += 1000;
		}
		// Maximum possible size of textures affects graphics quality
		score += (int)(physicalDeviceProperties.limits.maxImageDimension2D * 0.125f);
	
		return score;
	}
	
	int VulkanDevice::IsDeviceUsable(VulkanPhysicalDevice& inPhysicalDevice, SurfaceKHR& inSurface)
	{
		bool familiesSupported = inPhysicalDevice.SupportsQueueFamilies(QueueFlagBits::eGraphics | QueueFlagBits::eCompute);
		familiesSupported &= inPhysicalDevice.SupportsQueueFamily(inSurface); // present queue family should be supported
		bool extensionsSupported = inPhysicalDevice.SupportsExtensions(requiredDeviceExtensions);
		bool swapChainSupported = false;
		if (extensionsSupported)
		{
			swapChainSupported = inPhysicalDevice.SupportsSwapChain(inSurface);
		}
	
		return familiesSupported && extensionsSupported && swapChainSupported;
	}
	
}
