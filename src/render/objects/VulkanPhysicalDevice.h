#pragma once

#include "vulkan/vulkan.hpp"
#include <vector>
#include <map>
#include <set>
#include <optional>

namespace CGE
{

	using VULKAN_HPP_NAMESPACE::PresentModeKHR;
	using VULKAN_HPP_NAMESPACE::SurfaceFormatKHR;
	using VULKAN_HPP_NAMESPACE::SurfaceKHR;
	using VULKAN_HPP_NAMESPACE::SurfaceCapabilitiesKHR;
	using VULKAN_HPP_NAMESPACE::QueueFlags;
	using VULKAN_HPP_NAMESPACE::QueueFlagBits;
	using VULKAN_HPP_NAMESPACE::PhysicalDevice;
	using VULKAN_HPP_NAMESPACE::PhysicalDeviceType;
	using VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties;
	using VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties;
	using VULKAN_HPP_NAMESPACE::PhysicalDeviceFeatures;
	using VULKAN_HPP_NAMESPACE::PhysicalDeviceLimits;
	using VULKAN_HPP_NAMESPACE::ExtensionProperties;
	using VULKAN_HPP_NAMESPACE::QueueFamilyProperties;
	
	//------------------------------------------------------------------------------------------------------
	
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> computeFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> transferFamily;
		std::optional<uint32_t> sparseBindingFamily;
		std::optional<uint32_t> protectedFamily;
	};
	
	//------------------------------------------------------------------------------------------------------
	
	struct SwapChainSupportDetails
	{
		SurfaceCapabilitiesKHR capabilities;
		std::vector<SurfaceFormatKHR> formats;
		std::vector<PresentModeKHR> presentModes;
	
		bool IsUsable()
		{
			return !formats.empty() && !presentModes.empty();
		}
	};
	
	//------------------------------------------------------------------------------------------------------
	
	class VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice();
		VulkanPhysicalDevice(const PhysicalDevice& inDevice);
		virtual ~VulkanPhysicalDevice();
	
		PhysicalDevice& GetDevice() { return device; }
	
		const PhysicalDeviceProperties& GetProperties() const { return properties; }
		const PhysicalDeviceLimits& GetLimits() const { return properties.limits; }
		const PhysicalDeviceMemoryProperties& GetMemoryProperties() const { return memoryProperties; }
		const PhysicalDeviceFeatures& GetFeatures() const { return features; }
		const std::vector<QueueFamilyProperties>& GetQueueFamilyProperties() const;
	
		uint32_t GetQueueFamiliesCount() const;
		bool SupportsQueueFamilyAll(const QueueFamilyProperties& inQueueFamilyProperties, QueueFlags inFlags) const;
		bool SupportsQueueFamilyAll(uint32_t inIndex, QueueFlags inFlags) const;
		bool SupportsQueueFamilyAny(const QueueFamilyProperties& inQueueFamilyProperties, QueueFlags inFlags) const;
		bool SupportsQueueFamilyAny(uint32_t inIndex, QueueFlags inFlags) const;
		bool SupportsQueueFamily(uint32_t inIndex, SurfaceKHR inSurface) const;
		std::vector<uint32_t> FindFamiliesIndices(QueueFlags inFlags) const;
		const std::vector<uint32_t>& FindFamiliesIndices(SurfaceKHR inSurface);
		bool SupportsQueueFamilies(QueueFlags inFlags) const;
		bool SupportsQueueFamily(SurfaceKHR inSurface);
		bool SupportsQueueFamily(QueueFlagBits inFlag, uint32_t inMinAmount = 1);
		std::set<uint32_t> GetQueueFamiliesIndicesSet(QueueFlags inFlags);
		std::set<uint32_t> GetQueueFamiliesIndicesSet(QueueFlags inFlags, SurfaceKHR inSurface);
		QueueFamilyIndices GetQueueFamiliesIndices(QueueFlags inFlags, SurfaceKHR inSurface, bool inCacheResults = false);
		QueueFamilyIndices GetQueueFamiliesIndices(SurfaceKHR inSurface, bool inCacheResults = false);
		const QueueFamilyIndices& GetCachedQueueFamiliesIndices() const;
		std::optional<uint32_t> GetQueueFamilyIndex(QueueFlagBits inFlag, uint32_t inOrder = 0);
	
		bool IsDeviceType(PhysicalDeviceType inType);
		PhysicalDeviceType GetDeviceType() const;
	
		bool SupportsExtensions(const std::vector<const char*>& inExtensions);
		bool SupportsExtensions(const std::vector<std::string>& inExtensions);
		bool SupportsExtensions(std::set<std::string> inExtensions);
	
		bool SupportsSwapChain(SurfaceKHR inSurface);
		SwapChainSupportDetails QuerySwapChainSupport(SurfaceKHR inSurface);
	
		operator bool() const { return device; };
		operator PhysicalDevice() const { return device; };
	protected:
		PhysicalDevice device;
		PhysicalDeviceMemoryProperties memoryProperties;
		PhysicalDeviceProperties properties;
		PhysicalDeviceFeatures features;
		std::vector<ExtensionProperties> extensionProperties;
		std::vector<QueueFamilyProperties> queueFamilyProperties;
		QueueFamilyIndices cachedQueueFamilyProperties;
	
		std::map<QueueFlagBits, std::vector<uint32_t>> queueFamilyIndices;
		std::vector<uint32_t> presentFamilyIndices;
	
		void FillQueueFamiliesIndices();
	};
}
