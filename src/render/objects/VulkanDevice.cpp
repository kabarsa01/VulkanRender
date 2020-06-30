#include "VulkanDevice.h"
#include "core/Engine.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include "../memory/DeviceMemoryManager.h"

VulkanDevice::VulkanDevice()
{

}

VulkanDevice::~VulkanDevice()
{

}

void VulkanDevice::Create(const char* inAppName, const char* inEngine, bool inValidationEnabled, HWND inHwnd)
{
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

	InstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.setPApplicationInfo(&applicationInfo);
	instanceCreateInfo.setEnabledExtensionCount(glfwInstanceExtensionsCount);
	instanceCreateInfo.setPpEnabledExtensionNames(glfwExtensions);

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
	Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.setHwnd(inHwnd);
	surfaceCreateInfo.setHinstance(GetModuleHandle(nullptr));
	surface = instance.createWin32SurfaceKHR(surfaceCreateInfo);

	//-----------------------------------------------------------------------------------------------------

	std::vector<ExtensionProperties> extensionsResult = enumerateInstanceExtensionProperties();
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

	DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.setPQueueCreateInfos(queueCreateInfoVector.data());
	deviceCreateInfo.setQueueCreateInfoCount((uint32_t)queueCreateInfoVector.size());
	deviceCreateInfo.setPEnabledFeatures(&deviceFeatures);
	deviceCreateInfo.setEnabledExtensionCount((uint32_t)requiredExtensions.size());
	deviceCreateInfo.setPpEnabledExtensionNames(requiredExtensions.data());
	deviceCreateInfo.setEnabledLayerCount(0);

	device = physicalDevice.GetDevice().createDevice(deviceCreateInfo);

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
	std::vector<LayerProperties> layerProps = enumerateInstanceLayerProperties();

	for (std::string layer : validationLayers)
	{
		bool layerFound = false;
		for (LayerProperties prop : layerProps)
		{
			std::string availableLayerName(prop.layerName);
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
	bool extensionsSupported = inPhysicalDevice.SupportsExtensions(requiredExtensions);
	bool swapChainSupported = false;
	if (extensionsSupported)
	{
		swapChainSupported = inPhysicalDevice.SupportsSwapChain(inSurface);
	}

	return familiesSupported && extensionsSupported && swapChainSupported;
}

