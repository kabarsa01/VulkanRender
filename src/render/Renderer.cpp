#include "render/Renderer.h"
#include <iostream>
#include <map>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3native.h>
#include "core/Engine.h"
#include <vector>
#include <algorithm>


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
	CreateSwapChain();
	CreateImageViews();
}

void Renderer::RenderFrame()
{

}

void Renderer::Cleanup()
{
	for (const ImageView& imageView : swapChainImageViews)
	{
		vulkanDevice.destroyImageView(imageView);
	}

	vulkanDevice.destroySwapchainKHR(vulkanSwapChain);
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
	if (!IsDeviceUsable(inPhysicalDevice))
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

int Renderer::IsDeviceUsable(const VULKAN_HPP_NAMESPACE::PhysicalDevice& inPhysicalDevice)
{
	QueueFamilyIndices families = FindQueueFamilies(inPhysicalDevice);
	bool extensionsSupported = CheckPhysicalDeviceExtensionSupport(inPhysicalDevice);
	bool swapChainSupported = false;
	if (extensionsSupported)
	{
		swapChainSupported = QuerySwapChainSupport(inPhysicalDevice).IsUsable();
	}

	return families.IsComplete() && extensionsSupported && swapChainSupported;
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

	ResultValue<SurfaceCapabilitiesKHR> capabilitieResult = inPhysicalDevice.getSurfaceCapabilitiesKHR(vulkanSurface);
	if (capabilitieResult.result != Result::eSuccess)
		throw std::runtime_error("Error requesting surface capabilities");

	swapChainSupportDetails.capabilities = capabilitieResult.value;

	ResultValue<std::vector<SurfaceFormatKHR>> formatsResult = inPhysicalDevice.getSurfaceFormatsKHR(vulkanSurface);
	if (formatsResult.result != Result::eSuccess)
		throw std::runtime_error("Error requesting surface formats");

	swapChainSupportDetails.formats = formatsResult.value;

	ResultValue<std::vector<PresentModeKHR>> presentModesResult = inPhysicalDevice.getSurfacePresentModesKHR(vulkanSurface);
	if (presentModesResult.result != Result::eSuccess)
		throw std::runtime_error("Error requesting surface formats");

	swapChainSupportDetails.presentModes = presentModesResult.value;

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

SurfaceFormatKHR Renderer::ChooseSurfaceFormat(const std::vector<SurfaceFormatKHR>& inFormats)
{
	for (const SurfaceFormatKHR& surfaceFormat : inFormats)
	{
		if (surfaceFormat.format == Format::eR8G8B8A8Unorm && surfaceFormat.colorSpace == ColorSpaceKHR::eSrgbNonlinear)
		{
			return surfaceFormat;
		}
	}

	return inFormats[0];
}

PresentModeKHR Renderer::ChooseSwapChainPresentMode(const std::vector<PresentModeKHR>& inPresentModes)
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

VULKAN_HPP_NAMESPACE::Extent2D Renderer::ChooseSwapChainExtent(const VULKAN_HPP_NAMESPACE::SurfaceCapabilitiesKHR& inCapabilities)
{
	if (inCapabilities.currentExtent.width != UINT32_MAX)
	{
		return inCapabilities.currentExtent;
	}
	else
	{
		Extent2D extent;

		
		extent.setWidth( std::clamp<int>( width, inCapabilities.minImageExtent.width, inCapabilities.maxImageExtent.width ) );
		extent.setHeight( std::clamp<int>( height, inCapabilities.minImageExtent.height, inCapabilities.maxImageExtent.height ) );

		return extent;
	}
}

void Renderer::CreateSwapChain()
{
	SwapChainSupportDetails scSupportDetails = QuerySwapChainSupport(vulkanPhysicalDevice);

	SurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(scSupportDetails.formats);
	PresentModeKHR presentMode = ChooseSwapChainPresentMode(scSupportDetails.presentModes);
	Extent2D extent = ChooseSwapChainExtent(scSupportDetails.capabilities);

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	SurfaceCapabilitiesKHR& capabilities = scSupportDetails.capabilities;

	uint32_t imageCount = capabilities.minImageCount + 1;
	if ( capabilities.maxImageCount > 0 && (imageCount > capabilities.maxImageCount) )
	{
		imageCount = capabilities.maxImageCount;
	}

	SwapchainCreateInfoKHR createInfo;
	createInfo
		.setSurface(vulkanSurface)
		.setMinImageCount(imageCount)
		.setImageFormat(surfaceFormat.format)
		.setImageColorSpace(surfaceFormat.colorSpace)
		.setImageExtent(extent)
		.setImageUsage(ImageUsageFlagBits::eColorAttachment)// ImageUsageFlagBits::eTransferDst or ImageUsageFlagBits::eDepthStencilAttachment
		.setImageArrayLayers(1)
		.setPreTransform(capabilities.currentTransform)
		.setCompositeAlpha(CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(presentMode)
		.setClipped(VK_TRUE)
		.setOldSwapchain(SwapchainKHR());

	QueueFamilyIndices familyIndices = FindQueueFamilies(vulkanPhysicalDevice);
	uint32_t queueFamilyIndices[] = { familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value() };
	if (familyIndices.graphicsFamily != familyIndices.presentFamily)
	{
		createInfo.setImageSharingMode(SharingMode::eConcurrent);
		createInfo.setQueueFamilyIndexCount(2);
		createInfo.setPQueueFamilyIndices(queueFamilyIndices);
	}
	else
	{
		createInfo.setImageSharingMode(SharingMode::eExclusive);
	}

	ResultValue<SwapchainKHR> swapChainResult = vulkanDevice.createSwapchainKHR(createInfo);
	if (swapChainResult.result == Result::eSuccess) { vulkanSwapChain = swapChainResult.value; }
	else { throw std::runtime_error("Failed to create a swapchain"); }

	ResultValue<std::vector<Image>> imagesResult = vulkanDevice.getSwapchainImagesKHR(vulkanSwapChain);
	if (imagesResult.result == Result::eSuccess) { swapChainImages = imagesResult.value; }
	else { throw std::runtime_error("Failed to get swapchain images"); }
}

void Renderer::CreateImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());
	for (uint32_t index = 0; index < swapChainImages.size(); index++)
	{
		ImageViewCreateInfo createInfo;
		createInfo
			.setImage(swapChainImages[index])
			.setViewType(ImageViewType::e2D)
			.setFormat(swapChainImageFormat)
			.setComponents(ComponentMapping())
			.setSubresourceRange(ImageSubresourceRange(ImageAspectFlagBits::eColor, 0, 1, 0, 1));

		ResultValue<ImageView> imageViewResult = vulkanDevice.createImageView(createInfo);
		if (imageViewResult.result == Result::eSuccess) { swapChainImageViews[index] = imageViewResult.value; }
		else { throw std::runtime_error("Failed to create swapchain image view"); }
	}
}

void Renderer::CreateGraphicsPipeline()
{

}

