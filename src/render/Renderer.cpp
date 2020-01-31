#include "render/Renderer.h"
#include <iostream>
#include <map>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3native.h>
#include "core/Engine.h"
#include <vector>
#include <algorithm>
#include "shader/Shader.h"
#include "shader/ShaderModuleWrapper.h"
#include <array>


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
	vulkanInstance = createInstance(instanceCreateInfo);
	Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.setHwnd(glfwGetWin32Window(Engine::GetInstance()->GetGlfwWindow()));
	surfaceCreateInfo.setHinstance(GetModuleHandle(nullptr));
	vulkanSurface = vulkanInstance.createWin32SurfaceKHR(surfaceCreateInfo);

	//-----------------------------------------------------------------------------------------------------

	std::vector<ExtensionProperties> extensionsResult = enumerateInstanceExtensionProperties();
	for (ExtensionProperties extensionProperties : extensionsResult)
	{
		std::cout << "\t" << extensionProperties.extensionName << std::endl;
	}

	std::vector<PhysicalDevice> devices = vulkanInstance.enumeratePhysicalDevices();
	PickPhysicalDevice(devices);
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFramebuffers();
}

void Renderer::RenderFrame()
{

}

void Renderer::Cleanup()
{
	vulkanDevice.destroyCommandPool(commandPool);
	for (Framebuffer framebuffer : swapChainFramebuffers)
	{
		vulkanDevice.destroyFramebuffer(framebuffer);
	}
	vulkanDevice.destroyPipeline(pipeline);
	vulkanDevice.destroyPipelineLayout(pipelineLayout);
	vulkanDevice.destroyRenderPass(renderPass);
	for (const ImageView& imageView : swapChainImageViews) { vulkanDevice.destroyImageView(imageView); }
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

VULKAN_HPP_NAMESPACE::PhysicalDevice Renderer::GetPhysicalDevice()
{
	return vulkanPhysicalDevice;
}

VULKAN_HPP_NAMESPACE::Device Renderer::GetDevice()
{
	return vulkanDevice;
}

VULKAN_HPP_NAMESPACE::SwapchainKHR Renderer::GetSwapChain()
{
	return vulkanSwapChain;
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
	 std::vector<ExtensionProperties> extensionPropsResult = inPhysicalDevice.enumerateDeviceExtensionProperties();

	 std::set<std::string> requiredExtensionsSet(requiredExtensions.begin(), requiredExtensions.end());

	 for (ExtensionProperties& extension : extensionPropsResult)
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
		Bool32 surfaceSupportResult = inPhysicalDevice.getSurfaceSupportKHR(familyIndex, vulkanSurface);
		if (surfaceSupportResult)
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

	swapChainSupportDetails.capabilities = inPhysicalDevice.getSurfaceCapabilitiesKHR(vulkanSurface);
	swapChainSupportDetails.formats = inPhysicalDevice.getSurfaceFormatsKHR(vulkanSurface);
	swapChainSupportDetails.presentModes = inPhysicalDevice.getSurfacePresentModesKHR(vulkanSurface);

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

	vulkanDevice = vulkanPhysicalDevice.createDevice(deviceCreateInfo);

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

	vulkanSwapChain = vulkanDevice.createSwapchainKHR(createInfo);
	swapChainImages = vulkanDevice.getSwapchainImagesKHR(vulkanSwapChain);
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

		swapChainImageViews[index] = vulkanDevice.createImageView(createInfo);
	}
}

void Renderer::CreateRenderPass()
{
	AttachmentDescription colorAttachment;
	colorAttachment.setFormat(swapChainImageFormat);
	colorAttachment.setSamples(SampleCountFlagBits::e1);
	colorAttachment.setLoadOp(AttachmentLoadOp::eClear);
	colorAttachment.setStoreOp(AttachmentStoreOp::eStore);
	colorAttachment.setStencilLoadOp(AttachmentLoadOp::eDontCare);
	colorAttachment.setStencilStoreOp(AttachmentStoreOp::eDontCare);
	colorAttachment.setInitialLayout(ImageLayout::eUndefined);
	colorAttachment.setFinalLayout(ImageLayout::ePresentSrcKHR);

	AttachmentReference colorAttachmentRef;
	colorAttachmentRef.setAttachment(0);
	colorAttachmentRef.setLayout(ImageLayout::eColorAttachmentOptimal);

	SubpassDescription subpassDesc;
	subpassDesc.setPipelineBindPoint(PipelineBindPoint::eGraphics);
	subpassDesc.setColorAttachmentCount(1);
	subpassDesc.setPColorAttachments(&colorAttachmentRef); // layout(location = |index| ) out vec4 outColor references attachmant by index

	RenderPassCreateInfo renderPassInfo;
	renderPassInfo.setAttachmentCount(1);
	renderPassInfo.setPAttachments(&colorAttachment);
	renderPassInfo.setSubpassCount(1);
	renderPassInfo.setPSubpasses(&subpassDesc);

	renderPass = vulkanDevice.createRenderPass(renderPassInfo);
}

void Renderer::CreateGraphicsPipeline()
{
	Shader vertShader;
	vertShader.Load("content/shaders/BasicVert.spv");
	Shader fragShader;
	fragShader.Load("content/shaders/BasicFrag.spv");

	ShaderModuleWrapper vertShaderModule(vertShader);
	ShaderModuleWrapper fragShaderModule(fragShader);

	PipelineShaderStageCreateInfo vertStageInfo;
	vertStageInfo.setStage(ShaderStageFlagBits::eVertex);
	vertStageInfo.setModule(vertShaderModule.GetShaderModule());
	vertStageInfo.setPName("main");
	//vertStageInfo.setPSpecializationInfo(); spec info to set some constants

	PipelineShaderStageCreateInfo fragStageInfo;
	fragStageInfo.setStage(ShaderStageFlagBits::eFragment);
	fragStageInfo.setModule(fragShaderModule.GetShaderModule());
	fragStageInfo.setPName("main");

	std::vector<PipelineShaderStageCreateInfo> shaderStageInfoArray = { vertStageInfo, fragStageInfo };

	PipelineVertexInputStateCreateInfo vertexInputInfo;

	PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	inputAssemblyInfo.setTopology(PrimitiveTopology::eTriangleList);
	inputAssemblyInfo.setPrimitiveRestartEnable(VK_FALSE);

	Viewport viewport;
	viewport.setX(0.0f);
	viewport.setY(0.0f);
	viewport.setWidth((float)swapChainExtent.width);
	viewport.setHeight((float)swapChainExtent.height);
	viewport.setMinDepth(0.0f);
	viewport.setMaxDepth(1.0f);

	Rect2D scissor;
	scissor.setOffset(Offset2D(0, 0));
	scissor.setExtent(swapChainExtent);

	PipelineViewportStateCreateInfo viewportInfo;
	viewportInfo.setViewportCount(1);
	viewportInfo.setPViewports(&viewport);
	viewportInfo.setScissorCount(1);
	viewportInfo.setPScissors(&scissor);

	PipelineRasterizationStateCreateInfo rasterizationInfo;
	rasterizationInfo.setDepthClampEnable(VK_FALSE);
	rasterizationInfo.setRasterizerDiscardEnable(VK_FALSE);
	rasterizationInfo.setPolygonMode(PolygonMode::eFill);
	rasterizationInfo.setLineWidth(1.0f);
	rasterizationInfo.setCullMode(CullModeFlagBits::eBack);
	rasterizationInfo.setFrontFace(FrontFace::eClockwise);
	rasterizationInfo.setDepthBiasEnable(VK_FALSE);

	PipelineMultisampleStateCreateInfo multisampleInfo;

//	PipelineDepthStencilStateCreateInfo depthStencilInfo;

	PipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.setColorWriteMask(ColorComponentFlagBits::eR | ColorComponentFlagBits::eG | ColorComponentFlagBits::eB | ColorComponentFlagBits::eA);
	colorBlendAttachment.setBlendEnable(VK_FALSE);
	colorBlendAttachment.setSrcColorBlendFactor(BlendFactor::eOne);
	colorBlendAttachment.setDstColorBlendFactor(BlendFactor::eZero);
	colorBlendAttachment.setColorBlendOp(BlendOp::eAdd);
	colorBlendAttachment.setSrcAlphaBlendFactor(BlendFactor::eOne);
	colorBlendAttachment.setDstAlphaBlendFactor(BlendFactor::eZero);
	colorBlendAttachment.setAlphaBlendOp(BlendOp::eAdd);

	PipelineColorBlendStateCreateInfo colorBlendInfo;
	colorBlendInfo.setLogicOpEnable(VK_FALSE);
	colorBlendInfo.setLogicOp(LogicOp::eCopy);
	colorBlendInfo.setAttachmentCount(1);
	colorBlendInfo.setPAttachments(&colorBlendAttachment);
	colorBlendInfo.setBlendConstants( { 0.0f, 0.0f, 0.0f, 0.0f } );

	std::vector<DynamicState> dynamicStates = { DynamicState::eViewport, DynamicState::eLineWidth };
	PipelineDynamicStateCreateInfo dynamicStateInfo;
	dynamicStateInfo.setDynamicStateCount(2);
	dynamicStateInfo.setPDynamicStates(dynamicStates.data());

	PipelineLayoutCreateInfo layoutInfo;
	layoutInfo.setSetLayoutCount(0);
	layoutInfo.setPSetLayouts(nullptr);
	layoutInfo.setPushConstantRangeCount(0);
	layoutInfo.setPPushConstantRanges(nullptr);

	pipelineLayout = vulkanDevice.createPipelineLayout(layoutInfo);

	GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.setStageCount(2);
	pipelineInfo.setPStages(shaderStageInfoArray.data());
	pipelineInfo.setPVertexInputState(&vertexInputInfo);
	pipelineInfo.setPInputAssemblyState(&inputAssemblyInfo);
	pipelineInfo.setPViewportState(&viewportInfo);
	pipelineInfo.setPRasterizationState(&rasterizationInfo);
	pipelineInfo.setPMultisampleState(&multisampleInfo);
	pipelineInfo.setPDepthStencilState(nullptr);
	pipelineInfo.setPColorBlendState(&colorBlendInfo);
	pipelineInfo.setPDynamicState(&dynamicStateInfo);
	pipelineInfo.setLayout(pipelineLayout);
	pipelineInfo.setRenderPass(renderPass);
	pipelineInfo.setSubpass(0);
	pipelineInfo.setBasePipelineHandle(Pipeline());
	pipelineInfo.setBasePipelineIndex(-1);

	pipeline = vulkanDevice.createGraphicsPipeline(PipelineCache(), pipelineInfo);
}

void Renderer::CreateFramebuffers()
{
	swapChainFramebuffers.clear();

	for (int index = 0; index < swapChainImageViews.size(); index++)
	{
		ImageView attachments[] = { swapChainImageViews[index] };

		FramebufferCreateInfo framebufferInfo;
		framebufferInfo.setRenderPass(renderPass);
		framebufferInfo.setAttachmentCount(1);
		framebufferInfo.setPAttachments(attachments);
		framebufferInfo.setWidth(swapChainExtent.width);
		framebufferInfo.setHeight(swapChainExtent.height);
		framebufferInfo.setLayers(1);

		swapChainFramebuffers.push_back(vulkanDevice.createFramebuffer(framebufferInfo));
	}
}

void Renderer::CreateCommandPool()
{
	CommandPoolCreateInfo commandPoolInfo;
	commandPoolInfo.setQueueFamilyIndex(FindQueueFamilies(vulkanPhysicalDevice).graphicsFamily.value());
//	commandPoolInfo.setFlags(0);

	commandPool = vulkanDevice.createCommandPool(commandPoolInfo);

}

void Renderer::CreateCommandBuffers()
{
	commandBuffers.resize(swapChainFramebuffers.size());

	CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(commandPool);
	allocInfo.setLevel(CommandBufferLevel::ePrimary);
	allocInfo.setCommandBufferCount((uint32_t)commandBuffers.size());

	commandBuffers = vulkanDevice.allocateCommandBuffers(allocInfo);

	for (int index = 0; index < commandBuffers.size(); index++)
	{
		CommandBufferBeginInfo beginInfo;
//		beginInfo.setFlags(0);
		beginInfo.setPInheritanceInfo(nullptr);

		commandBuffers[index].begin(beginInfo);

		ClearValue clearValue;
		clearValue.setColor(ClearColorValue( std::array<float, 4>( { 0.0f, 0.0f, 0.0f, 1.0f } )));

		RenderPassBeginInfo passBeginInfo;
		passBeginInfo.setRenderPass(renderPass);
		passBeginInfo.setFramebuffer(swapChainFramebuffers[index]);
		passBeginInfo.setRenderArea(Rect2D( Offset2D(0,0), swapChainExtent ));
		passBeginInfo.setClearValueCount(1);
		passBeginInfo.setPClearValues(&clearValue);

		commandBuffers[index].beginRenderPass(passBeginInfo, SubpassContents::eInline);
		commandBuffers[index].bindPipeline(PipelineBindPoint::eGraphics, pipeline);
		commandBuffers[index].draw(3, 1, 0, 0);
		commandBuffers[index].endRenderPass();
		commandBuffers[index].end();
	}
}

