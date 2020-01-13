#include "render/Renderer.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <map>



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
	ApplicationInfo AppInfo(
		"SimpleVulkanRenderer",
		Version,
		"VulkanEngine",
		Version,
		VK_API_VERSION_1_0
	);

	uint32_t GlfwInstanceExtensionsCount;
	const char** GlfwExtensions = glfwGetRequiredInstanceExtensions(&GlfwInstanceExtensionsCount);

	InstanceCreateInfo InstCreateInfo;
	InstCreateInfo
		.setPApplicationInfo(&AppInfo)
		.setEnabledExtensionCount(GlfwInstanceExtensionsCount)
		.setPpEnabledExtensionNames(GlfwExtensions)
		.setEnabledLayerCount(0);

	//ResultValueType<VULKAN_HPP_NAMESPACE::Instance>::type Result = createInstance(InstCreateInfo, );
	ResultValue<Instance> InstanceResult = createInstance(InstCreateInfo);
	if (InstanceResult.result == Result::eSuccess)
	{
		VulkanInstance = InstanceResult.value;
	}

	ResultValue<std::vector<ExtensionProperties>> ExtensionsResult = enumerateInstanceExtensionProperties();
	if (ExtensionsResult.result == Result::eSuccess)
	{
		for (ExtensionProperties ExtensionProp : ExtensionsResult.value)
		{
			std::cout << "\t" << ExtensionProp.extensionName << std::endl;
		}
	}

	ResultValue<std::vector<PhysicalDevice>> DevicesResult = VulkanInstance.enumeratePhysicalDevices();
	PickPhysicalDevice(DevicesResult.value);
}

void Renderer::RenderFrame()
{

}

void Renderer::Cleanup()
{
	VulkanInstance.destroy();
}

void Renderer::SetResolution(int InWidth, int InHeight)
{

}

int Renderer::GetWidth() const
{
	return Width;
}

int Renderer::GetHeight() const
{
	return Height;
}

void Renderer::PickPhysicalDevice(std::vector<VULKAN_HPP_NAMESPACE::PhysicalDevice>& InDevices)
{
	// Use an ordered map to automatically sort candidates by increasing score
	std::multimap<int, PhysicalDevice> Candidates;

	for (const auto& DeviceElement : InDevices) {
		int Score = ScoreDeviceSuitability(DeviceElement);
		Candidates.insert(std::make_pair(Score, DeviceElement));
	}

	// Check if the best candidate is suitable at all
	if (Candidates.rbegin()->first > 0) {
		VulkanPhysicalDevice = Candidates.rbegin()->second;
	}
	else {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

int Renderer::ScoreDeviceSuitability(const VULKAN_HPP_NAMESPACE::PhysicalDevice& InPhysicalDevice)
{
	int Score = 0;

	PhysicalDeviceProperties PhysDeviceProperties = InPhysicalDevice.getProperties();
	// Discrete GPUs have a significant performance advantage
	if (PhysDeviceProperties.deviceType == PhysicalDeviceType::eDiscreteGpu) {
		Score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	Score += PhysDeviceProperties.limits.maxImageDimension2D * 0.5f;

	PhysicalDeviceFeatures PhysDeviceFeatures = InPhysicalDevice.getFeatures();
	// Application can't function without geometry shaders
	if (!PhysDeviceFeatures.geometryShader) {
		return 0;
	}

	return Score;
}

