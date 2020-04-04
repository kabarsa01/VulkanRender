#include "DeviceMemoryWrapper.h"
#include "core/Engine.h"

DeviceMemoryWrapper::DeviceMemoryWrapper()
{

}

DeviceMemoryWrapper::DeviceMemoryWrapper(const MemoryPropertyFlags& inMemPropertyFlags)
	: memPropertyFlags(inMemPropertyFlags)
{

}

DeviceMemoryWrapper::~DeviceMemoryWrapper()
{
	Free();
}

void DeviceMemoryWrapper::SetMemPropertyFlags(MemoryPropertyFlags inMemPropertyFlags)
{
	memPropertyFlags = inMemPropertyFlags;
}

MemoryPropertyFlags DeviceMemoryWrapper::GetMemPropertyFlags()
{
	return memPropertyFlags;
}

void DeviceMemoryWrapper::Allocate(const MemoryRequirements& inMemRequirements)
{
	size = inMemRequirements.size;
	memTypeBits = inMemRequirements.memoryTypeBits;
	AllocateInternal(inMemRequirements.size, inMemRequirements.memoryTypeBits);
}

void DeviceMemoryWrapper::Allocate()
{
	AllocateInternal(size, memTypeBits);
}

void DeviceMemoryWrapper::Free()
{
	if (deviceMemory)
	{
		device.freeMemory(deviceMemory);
		deviceMemory = nullptr;
	}
}

bool DeviceMemoryWrapper::IsValid()
{
	return deviceMemory;
}

void* DeviceMemoryWrapper::MapMemory(MemoryMapFlags inMapFlags, DeviceSize inMappingOffset, DeviceSize inMappingSize)
{
	mappedMem = device.mapMemory(deviceMemory, inMappingOffset, inMappingSize, inMapFlags);
	return mappedMem;
}

void* DeviceMemoryWrapper::GetMappedMem()
{
	return mappedMem;
}

void DeviceMemoryWrapper::UnmapMemory()
{
	device.unmapMemory(deviceMemory);
	mappedMem = nullptr;
}

void DeviceMemoryWrapper::CopyToMappedMem(size_t inDstOffset, const void* inSrcData, size_t inSrcOffset, size_t inSize)
{
	char* dst = reinterpret_cast<char*>(mappedMem);
	const char* srcData = reinterpret_cast<const char*>(inSrcData);
	memcpy(dst + inDstOffset, srcData + inSrcOffset, inSize);
}

void DeviceMemoryWrapper::MapCopyUnmap(MemoryMapFlags inMapFlags, DeviceSize inMappingOffset, DeviceSize inMappingSize, const void* inSrcData, size_t inCopyOffset, size_t inCopySize)
{
	MapMemory(inMapFlags, inMappingOffset, inMappingSize);
	CopyToMappedMem(0, inSrcData, inCopyOffset, inCopySize);
	UnmapMemory();
}

DeviceMemoryWrapper::operator bool() const
{
	return deviceMemory;
}

uint32_t DeviceMemoryWrapper::FindMemoryType(uint32_t inTypeFilter, MemoryPropertyFlags inPropFlags)
{
	PhysicalDeviceMemoryProperties memProps;
	memProps = Engine::GetRendererInstance()->GetPhysicalDevice().getMemoryProperties();

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

void DeviceMemoryWrapper::AllocateInternal(DeviceSize inSize, uint32_t inMemTypeBits)
{
	device = Engine::GetRendererInstance()->GetDevice();

	MemoryAllocateInfo memoryInfo;
	memoryInfo.setAllocationSize(inSize);
	memoryInfo.setMemoryTypeIndex(FindMemoryType(inMemTypeBits, memPropertyFlags));
	deviceMemory = device.allocateMemory(memoryInfo);
}

