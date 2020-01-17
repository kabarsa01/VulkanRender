#include "render/Renderer.h"
#include <iostream>
#include <map>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3native.h>
#include "core/Engine.h"
#include <vector>


using namespace VULKAN_HPP_NAMESPACE;

Renderer::Renderer()
{

}

Renderer::~Renderer()
{

}

void Renderer::OnInitialize()
{

}

void Renderer::Init()
{
	ApplicationInfo applicationInfo(
		"SimpleVulkanRenderer",
		version,
		"VulkanEngine",
		version,
		VK_API_VERSION_1_0
	);

	uint32_t glfwInstanceExtensionsCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionsCount);

	InstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo
		.setPApplicationInfo(&applicationInfo)
		.setEnabledExtensionCount(glfwInstanceExtensionsCount)
		.setPpEnabledExtensionNames(glfwExtensions)
		.setEnabledLayerCount(0);

	//ResultValueType<VULKAN_HPP_NAMESPACE::Instance>::type Result = createInstance(InstCreateInfo, );
	ResultValue<Instance> instanceResult = createInstance(instanceCreateInfo);
	if (instanceResult.result == Result::eSuccess)
	{
		vulkanInstance = instanceResult.value;
	}

	Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.setHwnd(glfwGetWin32Window(Engine::GetInstance()->GetGlfwWindow()));
	surfaceCreateInfo.setHinstance(GetModuleHandle(nullptr));
	ResultValue<SurfaceKHR> surfaceResult = vulkanInstance.createWin32SurfaceKHR(surfaceCreateInfo);

	if (surfaceResult.result == Result::eSuccess) { vulkanSurface = surfaceResult.value; }
	else { throw std::runtime_error("failed to create surface!"); }

	//-----------------------------------------------------------------------------------------------------

	ResultValue<std::vector<ExtensionProperties>> extensionsResult = enumerateInstanceExtensionProperties();
	if (extensionsResult.result == Result::eSuccess)
	{
		for (ExtensionProperties extensionProperties : extensionsResult.value)
		{
			std::cout << "\t" << extensionProperties.extensionName << std::endl;
		}
	}

	ResultValue<std::vector<PhysicalDevice>> devicesResult = vulkanInstance.enumeratePhysicalDevices();
	PickPhysicalDevice(devicesResult.value);
	CreateLogicalDevice();
}

void Renderer::RenderFrame()
{

}

void Renderer::Cleanup()
{
	vulkanDevice.destroy();
	vulkanInstance.destroySurfaceKHR(vulkanSurface);
	vulkanInstance.destroy();
}

void Renderer::SetResolution(int inWidth, int inHeight)
{
	width = inWidth;
	height = inHeight;
}

int Renderer::GetWidth() const
{
	return width;
}

int Renderer::GetHeight() const
{
	return height;
}

void Renderer::PickPhysicalDevice(std::vector<VULKAN_HPP_NAMESPACE::PhysicalDevice>& inDevices)
{
	// Use an ordered map to automatically sort candidates by increasing score
	std::multimap<int, PhysicalDevice> candidates;

	for (const auto& device : inDevices) {
		int score = ScoreDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	// Check if the best candidate is suitable at all
	if (candidates.rbegin()->first > 0) {
		vulkanPhysicalDevice = candidates.rbegin()->second;
	}
	else {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

int Renderer::ScoreDeviceSuitability(const VULKAN_HPP_NAMESPACE::PhysicalDevice& inPhysicalDevice)
{
	QueueFamilyIndices families = FindQueueFamilies(inPhysicalDevice);
	if (!families.IsComplete() || !CheckPhysicalDeviceExtensionSupport(inPhysicalDevice))
	{
		return 0;
	}

	int score = 0;

	PhysicalDeviceProperties physicalDeviceProperties = inPhysicalDevice.getProperties();
	// Discrete GPUs have a significant performance advantage
	if (physicalDeviceProperties.deviceType == PhysicalDeviceType::eDiscreteGpu) {
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	score += (int) ( physicalDeviceProperties.limits.maxImageDimension2D * 0.5f );

	PhysicalDeviceFeatures physicalDeviceFeatures = inPhysicalDevice.getFeatures();
	// Application can't function without geometry shaders
	if (!physicalDeviceFeatures.geometryShader) {
		return 0;
	}

	return score;
}

bool Renderer::CheckPhysicalDeviceExtensionSupport(const VULKAN_HPP_NAMESPACE::PhysicalDevice& inPhysicalDevice)
{
	 ResultValue<std::vector<ExtensionProperties>> extensionPropsResult = inPhysicalDevice.enumerateDeviceExtensionProperties();
	 if (extensionPropsResult.result != Result::eSuccess)
	 {
		 return false;
	 }

	 std::set<std::string> requiredExtensionsSet(requiredExtensions.begin(), requiredExtensions.end());

	 for (ExtensionProperties& extension : extensionPropsResult.value)
	 {
		 requiredExtensionsSet.erase(extension.extensionName);
	 }

	 return requiredExtensionsSet.empty();
}

QueueFamilyIndices Renderer::FindQueueFamilies(const VULKAN_HPP_NAMESPACE::PhysicalDevice& inPhysicalDevice)
{
	QueueFamilyIndices indices;

	uint32_t familyIndex = 0;
	std::vector<QueueFamilyProperties> queueFamiliesProperties = inPhysicalDevice.getQueueFamilyProperties();
	for (QueueFamilyProperties & familyProperties : queueFamiliesProperties)
	{
		if (familyProperties.queueFlags & QueueFlagBits::eGraphics)
		{
			indices.graphicsFamily = familyIndex;
		}
		if (familyProperties.queueFlags & QueueFlagBits::eCompute)
		{
			indices.computeFamily = familyIndex;
		}
		ResultValue<Bool32> surfaceSupportResult = inPhysicalDevice.getSurfaceSupportKHR(familyIndex, vulkanSurface);
		if (surfaceSupportResult.result == Result::eSuccess && surfaceSupportResult.value)
		{
			indices.presentFamily = familyIndex;
		}

		familyIndex++;
	}

	return indices;
}

SwapChainSupportDetails Renderer::QuerySwapChainSupport(const VULKAN_HPP_NAMESPACE::PhysicalDevice& inPhysicalDevice)
{
	SwapChainSupportDetails swapChainSupportDetails;



	return swapChainSupportDetails;
}

void Renderer::CreateLogicalDevice()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(vulkanPhysicalDevice);

	std::vector<DeviceQueueCreateInfo> queueCreateInfoVector;
	for (uint32_t queueFamiltIndex : queueFamilyIndices.GetFamiliesSet())
	{
		DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.setQueueFamilyIndex(queueFamiltIndex);
		queueCreateInfo.setQueueCount(1); // magic 1
		float priority;
		queueCreateInfo.setPQueuePriorities(&priority);

		queueCreateInfoVector.push_back(queueCreateInfo);
	}

	PhysicalDeviceFeatures deviceFeatures;

	DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.setPQueueCreateInfos(queueCreateInfoVector.data());
	deviceCreateInfo.setQueueCreateInfoCount((uint32_t)queueCreateInfoVector.size());
	deviceCreateInfo.setPEnabledFeatures(&deviceFeatures);
	deviceCreateInfo.setEnabledExtensionCount((uint32_t)requiredExtensions.size());
	deviceCreateInfo.setPpEnabledExtensionNames(requiredExtensions.data());
	deviceCreateInfo.setEnabledLayerCount(0);

	ResultValue<Device> deviceResult = vulkanPhysicalDevice.createDevice(deviceCreateInfo);
	if (deviceResult.result == Result::eSuccess)
	{
		vulkanDevice = deviceResult.value;
	}
	else
	{
		throw std::runtime_error("failed to create device!");
	}

	graphicsQueue = vulkanDevice.getQueue(queueFamilyIndices.graphicsFamily.value(), 0);
	computeQueue = vulkanDevice.getQueue(queueFamilyIndices.computeFamily.value(), 0);
	presentQueue = vulkanDevice.getQueue(queueFamilyIndices.presentFamily.value(), 0);
}

