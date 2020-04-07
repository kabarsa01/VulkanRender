#include "DeviceMemoryWrapper.h"
#include "core/Engine.h"

DeviceMemoryWrapper::DeviceMemoryWrapper(bool inScoped)
	: scoped(inScoped)
{

}

DeviceMemoryWrapper::DeviceMemoryWrapper(const MemoryPropertyFlags& inMemPropertyFlags, bool inScoped)
	: memPropertyFlags(inMemPropertyFlags)
	, scoped(inScoped)
{

}

DeviceMemoryWrapper::~DeviceMemoryWrapper()
{
	if (scoped)
	{
		Free();
	}
}

DeviceMemoryWrapper& DeviceMemoryWrapper::SetSize(DeviceSize inSize)
{
	size = inSize;
	return *this;
}

DeviceMemoryWrapper& DeviceMemoryWrapper::SetMemTypeBits(uint32_t inMemTypeBits)
{
	memTypeBits = inMemTypeBits;
	return *this;
}

DeviceMemoryWrapper& DeviceMemoryWrapper::SetMemPropertyFlags(MemoryPropertyFlags inMemPropertyFlags)
{
	memPropertyFlags = inMemPropertyFlags;
	return *this;
}

MemoryPropertyFlags DeviceMemoryWrapper::GetMemPropertyFlags()
{
	return memPropertyFlags;
}

void DeviceMemoryWrapper::Allocate(DeviceSize inSize, uint32_t inMemTypeBits, MemoryPropertyFlags inMemPropertyFlags)
{
	device = Engine::GetRendererInstance()->GetDevice();

	MemoryAllocateInfo memoryInfo;
	memoryInfo.setAllocationSize(inSize);
	memoryInfo.setMemoryTypeIndex(FindMemoryTypeStatic(inMemTypeBits, inMemPropertyFlags));
	deviceMemory = device.allocateMemory(memoryInfo);

	size = inSize;
	memTypeBits = inMemTypeBits;
	memPropertyFlags = inMemPropertyFlags;
}

void DeviceMemoryWrapper::Allocate(const MemoryRequirements& inMemRequirements, MemoryPropertyFlags inMemPropertyFlags)
{
	if (!deviceMemory)
	{
		Allocate(inMemRequirements.size, inMemRequirements.memoryTypeBits, inMemPropertyFlags);
	}
}

void DeviceMemoryWrapper::Allocate()
{
	if (!deviceMemory)
	{
		Allocate(size, memTypeBits, memPropertyFlags);
	}
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

DeviceMemoryWrapper::operator DeviceMemory() const
{
	return deviceMemory;
}

uint32_t DeviceMemoryWrapper::FindMemoryTypeStatic(uint32_t inTypeFilter, MemoryPropertyFlags inPropFlags)
{
	PhysicalDeviceMemoryProperties memProps = Engine::GetRendererInstance()->GetPhysicalDeviceMemoryProps();

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



