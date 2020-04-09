#include "VulkanPhysicalDevice.h"

VulkanPhysicalDevice::VulkanPhysicalDevice()
{

}

VulkanPhysicalDevice::VulkanPhysicalDevice(const PhysicalDevice& inDevice)
	: device(inDevice)
{
	// caching mem props to win some time
	memoryProperties = device.getMemoryProperties();
	properties = device.getProperties();
	features = device.getFeatures();
	queueFamilyProperties = device.getQueueFamilyProperties();
	extensionProperties = device.enumerateDeviceExtensionProperties();

	FillQueueFamiliesIndices();
}

void VulkanPhysicalDevice::FillQueueFamiliesIndices()
{
	queueFamilyIndices[QueueFlagBits::eGraphics] = FindFamiliesIndices(QueueFlagBits::eGraphics);
	queueFamilyIndices[QueueFlagBits::eCompute] = FindFamiliesIndices(QueueFlagBits::eCompute);
	queueFamilyIndices[QueueFlagBits::eTransfer] = FindFamiliesIndices(QueueFlagBits::eTransfer);
	queueFamilyIndices[QueueFlagBits::eSparseBinding] = FindFamiliesIndices(QueueFlagBits::eSparseBinding);
	queueFamilyIndices[QueueFlagBits::eProtected] = FindFamiliesIndices(QueueFlagBits::eProtected);
}

VulkanPhysicalDevice::~VulkanPhysicalDevice()
{

}

const std::vector<QueueFamilyProperties>& VulkanPhysicalDevice::GetQueueFamilyProperties() const
{
	return queueFamilyProperties;
}

uint32_t VulkanPhysicalDevice::GetQueueFamiliesCount() const
{
	return static_cast<uint32_t>(queueFamilyProperties.size());
}

bool VulkanPhysicalDevice::SupportsQueueFamilyAll(const QueueFamilyProperties& inQueueFamilyProperties, QueueFlags inFlags) const
{
	return (inQueueFamilyProperties.queueFlags & inFlags) == inFlags;
}

bool VulkanPhysicalDevice::SupportsQueueFamilyAll(uint32_t inIndex, QueueFlags inFlags) const
{
	return SupportsQueueFamilyAll(queueFamilyProperties[inIndex], inFlags);
}

bool VulkanPhysicalDevice::SupportsQueueFamilyAny(const QueueFamilyProperties& inQueueFamilyProperties, QueueFlags inFlags) const
{
	return (inQueueFamilyProperties.queueFlags & inFlags) != QueueFlags();
}

bool VulkanPhysicalDevice::SupportsQueueFamilyAny(uint32_t inIndex, QueueFlags inFlags) const
{
	return SupportsQueueFamilyAny(queueFamilyProperties[inIndex], inFlags);
}

bool VulkanPhysicalDevice::SupportsQueueFamily(uint32_t inIndex, SurfaceKHR inSurface) const
{
	return device.getSurfaceSupportKHR(inIndex, inSurface);
}

std::vector<uint32_t> VulkanPhysicalDevice::FindFamiliesIndices(QueueFlags inFlags) const
{
	std::vector<uint32_t> familiesIndices;
	for (uint32_t index = 0; index < queueFamilyProperties.size(); index++)
	{
		if (SupportsQueueFamilyAll(index, inFlags)) familiesIndices.push_back(index);
	}
	return familiesIndices;
}

const std::vector<uint32_t>& VulkanPhysicalDevice::FindFamiliesIndices(SurfaceKHR inSurface)
{
	if (presentFamilyIndices.size() == 0)
	{
		for (uint32_t index = 0; index < queueFamilyProperties.size(); index++)
		{
			if (SupportsQueueFamily(index, inSurface)) presentFamilyIndices.push_back(index);
		}
	}

	return presentFamilyIndices;
}

bool VulkanPhysicalDevice::SupportsQueueFamilies(QueueFlags inFlags) const
{
	for (uint32_t index = 0; index < queueFamilyProperties.size(); index++)
	{
		QueueFlags sharedFlags = queueFamilyProperties[index].queueFlags & inFlags;
		inFlags &= ~sharedFlags;
	}

	return inFlags == QueueFlags();
}

bool VulkanPhysicalDevice::SupportsQueueFamily(SurfaceKHR inSurface)
{
	return FindFamiliesIndices(inSurface).size() > 0;
}

bool VulkanPhysicalDevice::SupportsQueueFamily(QueueFlagBits inFlag, uint32_t inMinAmount)
{
	return queueFamilyIndices[inFlag].size() >= inMinAmount;
}

std::set<uint32_t> VulkanPhysicalDevice::GetQueueFamiliesIndicesSet(QueueFlags inFlags)
{
	std::set<uint32_t> indices;
	for (const std::pair<QueueFlagBits, std::vector<uint32_t>>& pair : queueFamilyIndices)
	{
		if ( ((pair.first & inFlags) != QueueFlags()) && (pair.second.size() > 0) )
		{
			indices.insert(pair.second[0]);
			inFlags &= ~pair.first;
		}
	}

	return indices;
}

std::set<uint32_t> VulkanPhysicalDevice::GetQueueFamiliesIndicesSet(QueueFlags inFlags, SurfaceKHR inSurface)
{
	std::set<uint32_t> indices = GetQueueFamiliesIndicesSet(inFlags);
	const std::vector<uint32_t>& presentIndices = FindFamiliesIndices(inSurface);
	if (presentIndices.size() > 0)
	{
		indices.insert(presentIndices[0]);
	}
	return indices;
}

QueueFamilyIndices VulkanPhysicalDevice::GetQueueFamiliesIndices(QueueFlags inFlags, SurfaceKHR inSurface, bool inCacheResults)
{
	QueueFamilyIndices indices;

	indices.graphicsFamily = (inFlags & QueueFlagBits::eGraphics) ? GetQueueFamilyIndex(QueueFlagBits::eGraphics) : std::nullopt;
	indices.computeFamily = (inFlags & QueueFlagBits::eCompute) ? GetQueueFamilyIndex(QueueFlagBits::eCompute) : std::nullopt;
	indices.transferFamily = (inFlags & QueueFlagBits::eTransfer) ? GetQueueFamilyIndex(QueueFlagBits::eTransfer) : std::nullopt;
	indices.sparseBindingFamily = (inFlags & QueueFlagBits::eSparseBinding) ? GetQueueFamilyIndex(QueueFlagBits::eSparseBinding) : std::nullopt;
	indices.protectedFamily = (inFlags & QueueFlagBits::eProtected) ? GetQueueFamilyIndex(QueueFlagBits::eProtected) : std::nullopt;

	const std::vector<uint32_t>& presentIndices = FindFamiliesIndices(inSurface);
	indices.presentFamily = presentIndices.size() > 0 ? std::optional<uint32_t>(presentIndices[0]) : std::nullopt;

	if (inCacheResults)
	{
		cachedQueueFamilyProperties = indices;
	}

	return indices;
}

QueueFamilyIndices VulkanPhysicalDevice::GetQueueFamiliesIndices(SurfaceKHR inSurface, bool inCacheResults)
{
	return GetQueueFamiliesIndices(
		QueueFlagBits::eGraphics      |
		QueueFlagBits::eCompute       |
		QueueFlagBits::eTransfer      |
		QueueFlagBits::eSparseBinding |
		QueueFlagBits::eProtected,
		inSurface,
		inCacheResults
	);
}

const QueueFamilyIndices& VulkanPhysicalDevice::GetCachedQueueFamiliesIndices() const
{
	return cachedQueueFamilyProperties;
}

std::optional<uint32_t> VulkanPhysicalDevice::GetQueueFamilyIndex(QueueFlagBits inFlag, uint32_t inOrder)
{
	return SupportsQueueFamily(inFlag, inOrder + 1) ? std::optional<uint32_t>(queueFamilyIndices[inFlag][inOrder]) : std::nullopt;
}

bool VulkanPhysicalDevice::IsDeviceType(PhysicalDeviceType inType)
{
	return properties.deviceType == inType;
}

PhysicalDeviceType VulkanPhysicalDevice::GetDeviceType() const
{
	return properties.deviceType;
}

bool VulkanPhysicalDevice::SupportsExtensions(const std::vector<const char*>& inExtensions)
{
	return SupportsExtensions(std::set<std::string>(inExtensions.begin(), inExtensions.end()));
}

bool VulkanPhysicalDevice::SupportsExtensions(const std::vector<std::string>& inExtensions)
{
	return SupportsExtensions(std::set<std::string>(inExtensions.begin(), inExtensions.end()));
}

bool VulkanPhysicalDevice::SupportsExtensions(std::set<std::string> inExtensions)
{
	for (ExtensionProperties& extension : extensionProperties)
	{
		inExtensions.erase(extension.extensionName);
	}

	return inExtensions.empty();
}

bool VulkanPhysicalDevice::SupportsSwapChain(SurfaceKHR inSurface)
{
	return QuerySwapChainSupport(inSurface).IsUsable();
}

SwapChainSupportDetails VulkanPhysicalDevice::QuerySwapChainSupport(SurfaceKHR inSurface)
{
	SwapChainSupportDetails swapChainSupportDetails;

	swapChainSupportDetails.capabilities = device.getSurfaceCapabilitiesKHR(inSurface);
	swapChainSupportDetails.formats = device.getSurfaceFormatsKHR(inSurface);
	swapChainSupportDetails.presentModes = device.getSurfacePresentModesKHR(inSurface);

	return swapChainSupportDetails;
}

