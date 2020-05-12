#include "VulkanDeviceMemory.h"
#include "core/Engine.h"
#include "../Renderer.h"

VulkanDeviceMemory::VulkanDeviceMemory(bool inScoped)
	: scoped(inScoped)
	, deviceMemory(nullptr)
{

}

VulkanDeviceMemory::VulkanDeviceMemory(const MemoryPropertyFlags& inMemPropertyFlags, bool inScoped)
	: memPropertyFlags(inMemPropertyFlags)
	, scoped(inScoped)
	, deviceMemory(nullptr)
{

}

VulkanDeviceMemory::~VulkanDeviceMemory()
{
	if (scoped)
	{
		Free();
	}
}

VulkanDeviceMemory& VulkanDeviceMemory::SetSize(DeviceSize inSize)
{
	size = inSize;
	return *this;
}

VulkanDeviceMemory& VulkanDeviceMemory::SetMemTypeBits(uint32_t inMemTypeBits)
{
	memTypeBits = inMemTypeBits;
	return *this;
}

VulkanDeviceMemory& VulkanDeviceMemory::SetMemPropertyFlags(MemoryPropertyFlags inMemPropertyFlags)
{
	memPropertyFlags = inMemPropertyFlags;
	return *this;
}

MemoryPropertyFlags VulkanDeviceMemory::GetMemPropertyFlags()
{
	return memPropertyFlags;
}

void VulkanDeviceMemory::Allocate(DeviceSize inSize, uint32_t inMemTypeBits, MemoryPropertyFlags inMemPropertyFlags)
{
	device = Engine::GetRendererInstance()->GetVulkanDevice();

	MemoryAllocateInfo memoryInfo;
	memoryInfo.setAllocationSize(inSize);
	memoryInfo.setMemoryTypeIndex(FindMemoryTypeStatic(inMemTypeBits, inMemPropertyFlags));
	deviceMemory = device.allocateMemory(memoryInfo);

	size = inSize;
	memTypeBits = inMemTypeBits;
	memPropertyFlags = inMemPropertyFlags;
}

void VulkanDeviceMemory::Allocate(const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags)
{
	if (!deviceMemory)
	{
		Allocate(inMemRequirements.size, inMemRequirements.memoryTypeBits, inMemPropertyFlags);
	}
}

void VulkanDeviceMemory::Allocate()
{
	if (!deviceMemory)
	{
		Allocate(size, memTypeBits, memPropertyFlags);
	}
}

void VulkanDeviceMemory::Free()
{
	if (deviceMemory)
	{
		device.freeMemory(deviceMemory);
		deviceMemory = nullptr;
	}
}

bool VulkanDeviceMemory::IsValid()
{
	return deviceMemory;
}

void* VulkanDeviceMemory::MapMemory(MemoryMapFlags inMapFlags, DeviceSize inMappingOffset, DeviceSize inMappingSize)
{
	mappedMem = device.mapMemory(deviceMemory, inMappingOffset, inMappingSize, inMapFlags);
	return mappedMem;
}

void* VulkanDeviceMemory::GetMappedMem()
{
	return mappedMem;
}

void VulkanDeviceMemory::UnmapMemory()
{
	device.unmapMemory(deviceMemory);
	mappedMem = nullptr;
}

void VulkanDeviceMemory::CopyToMappedMem(size_t inDstOffset, const void* inSrcData, size_t inSrcOffset, size_t inSize)
{
	char* dst = reinterpret_cast<char*>(mappedMem);
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
	return deviceMemory;
}

VulkanDeviceMemory::operator DeviceMemory() const
{
	return deviceMemory;
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



