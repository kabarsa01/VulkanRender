#include "VulkanDeviceMemory.h"
#include "core/Engine.h"
#include "../Renderer.h"

namespace CGE
{
	namespace vk = VULKAN_HPP_NAMESPACE;

	VulkanDeviceMemory::VulkanDeviceMemory(bool inScoped)
		: m_scoped(inScoped)
		, m_deviceMemory(nullptr)
		, m_deviceLocal(true)
	{
	
	}
	
	VulkanDeviceMemory::~VulkanDeviceMemory()
	{
		if (m_scoped)
		{
			Free();
		}
	}
	
	VulkanDeviceMemory& VulkanDeviceMemory::SetSize(DeviceSize inSize)
	{
		m_size = inSize;
		return *this;
	}
	
	VulkanDeviceMemory& VulkanDeviceMemory::SetRequirements(const vk::MemoryRequirements& memRequirements)
	{
		m_requirements = memRequirements;
		return *this;
	}
	
	VulkanDeviceMemory& VulkanDeviceMemory::SetPropertyFlags(MemoryPropertyFlags inMemPropertyFlags)
	{
		m_propertyFlags = inMemPropertyFlags;
		return *this;
	}
	
	void VulkanDeviceMemory::Allocate()
	{
		if (m_deviceMemory)
		{
			return;
		}
		m_nativeDevice = Engine::GetRendererInstance()->GetVulkanDevice();

		vk::MemoryAllocateFlagsInfo flagsInfo = {};
		flagsInfo.setFlags(vk::MemoryAllocateFlagBits::eDeviceAddress);

		vk::MemoryAllocateInfo memoryInfo;
		memoryInfo.setAllocationSize(m_size);
		memoryInfo.setMemoryTypeIndex(FindMemoryTypeStatic(m_requirements.memoryTypeBits, m_propertyFlags));
		memoryInfo.pNext = &flagsInfo;
		m_deviceMemory = m_nativeDevice.allocateMemory(memoryInfo);
	}
	
	void VulkanDeviceMemory::Free()
	{
		if (m_deviceMemory)
		{
			m_nativeDevice.freeMemory(m_deviceMemory);
			m_deviceMemory = nullptr;
		}
	}
	
	bool VulkanDeviceMemory::IsValid()
	{
		return m_deviceMemory;
	}
	
	void* VulkanDeviceMemory::MapMemory(MemoryMapFlags inMapFlags, DeviceSize inMappingOffset, DeviceSize inMappingSize)
	{
		m_mappedMem = m_nativeDevice.mapMemory(m_deviceMemory, inMappingOffset, inMappingSize, inMapFlags);
		return m_mappedMem;
	}
	
	void* VulkanDeviceMemory::GetMappedMem()
	{
		return m_mappedMem;
	}
	
	void VulkanDeviceMemory::UnmapMemory()
	{
		m_nativeDevice.unmapMemory(m_deviceMemory);
		m_mappedMem = nullptr;
	}
	
	void VulkanDeviceMemory::CopyToMappedMem(size_t inDstOffset, const void* inSrcData, size_t inSrcOffset, size_t inSize)
	{
		char* dst = reinterpret_cast<char*>(m_mappedMem);
		const char* srcData = reinterpret_cast<const char*>(inSrcData);
		memcpy(dst + inDstOffset, srcData + inSrcOffset, inSize);
	}
	
	void VulkanDeviceMemory::MapCopyUnmap(MemoryMapFlags inMapFlags, DeviceSize inMappingOffset, DeviceSize inMappingSize, const void* inSrcData, size_t inCopyOffset, size_t inCopySize)
	{
		MapMemory(inMapFlags, inMappingOffset, inMappingSize);
		CopyToMappedMem(0, inSrcData, inCopyOffset, inCopySize);
		UnmapMemory();
	}
	
	VulkanDeviceMemory::operator bool() const
	{
		return m_deviceMemory;
	}
	
	VulkanDeviceMemory::operator DeviceMemory() const
	{
		return m_deviceMemory;
	}
	
	uint32_t VulkanDeviceMemory::FindMemoryTypeStatic(uint32_t inTypeFilter, MemoryPropertyFlags inPropFlags)
	{
		PhysicalDeviceMemoryProperties memProps = Engine::GetRendererInstance()->GetVulkanDevice().GetPhysicalDevice().GetMemoryProperties();
	
		for (uint32_t index = 0; index < memProps.memoryTypeCount; index++)
		{
			bool propFlagsSufficient = (memProps.memoryTypes[index].propertyFlags & inPropFlags) == inPropFlags;
			bool hasTheType = inTypeFilter & (1 << index);
			if (hasTheType && propFlagsSufficient)
			{
				return index;
			}
		}
	
		throw std::runtime_error("No suitable memory type found");
	}
	
	
	
}
