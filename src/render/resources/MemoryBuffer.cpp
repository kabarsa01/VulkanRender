#include "render/resources/MemoryBuffer.h"
#include "core/Engine.h"

using namespace VULKAN_HPP_NAMESPACE;

MemoryBuffer::MemoryBuffer()
{
	usageFlags = BufferUsageFlagBits::eStorageBuffer;
	memPropertyFlags = MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent;
}

MemoryBuffer::~MemoryBuffer()
{
	Destroy();
}

Buffer MemoryBuffer::GetBuffer()
{
	return buffer;
}

DeviceMemory MemoryBuffer::GetDeviceMemory()
{
	return deviceMemory;
}

void MemoryBuffer::Create()
{
	BufferCreateInfo bufferInfo;
	bufferInfo.setSharingMode(SharingMode::eExclusive);
	//	bufferInfo.setPQueueFamilyIndices() for concurent mode
	bufferInfo.setSize(size);
	bufferInfo.setUsage(usageFlags);
	//	bufferInfo.setFlags(BufferCreateFlagBits::eSparseBinding);

	Device device = Engine::GetRendererInstance()->GetDevice();
	buffer = device.createBuffer(bufferInfo);

	MemoryRequirements memRequirements = device.getBufferMemoryRequirements(buffer);

	MemoryAllocateInfo memoryInfo;
	memoryInfo.setAllocationSize(memRequirements.size);
	memoryInfo.setMemoryTypeIndex(FindMemoryType(memRequirements.memoryTypeBits, memPropertyFlags));
	deviceMemory = device.allocateMemory(memoryInfo);
	device.bindBufferMemory(buffer, deviceMemory, 0);
}

void MemoryBuffer::Destroy()
{
	Device device = Engine::GetRendererInstance()->GetDevice();
	if (buffer)
	{
		device.destroyBuffer(buffer);
		buffer = nullptr;
	}
	if (deviceMemory)
	{
		device.freeMemory(deviceMemory);
		deviceMemory = nullptr;
	}
}

void* MemoryBuffer::MapMemory(MemoryMapFlags inMapFlags, DeviceSize inOffset)
{
	Device device = Engine::GetRendererInstance()->GetDevice();
	return device.mapMemory(deviceMemory, inOffset, size, inMapFlags);
}

void MemoryBuffer::UnmapMemory()
{
	Device device = Engine::GetRendererInstance()->GetDevice();
	device.unmapMemory(deviceMemory);
}

void MemoryBuffer::CopyData(const void* inSrcData, MemoryMapFlags inMapFlags, DeviceSize inOffset)
{
	void* data = MapMemory(inMapFlags, inOffset);
	memcpy(data, inSrcData, size);
	UnmapMemory();
}

void MemoryBuffer::CopyBuffer(MemoryBuffer& inSrc, MemoryBuffer& inDst)
{
	CommandBufferAllocateInfo commandAllocInfo;
	commandAllocInfo.setCommandPool(Engine::GetRendererInstance()->GetCommandPool());
	commandAllocInfo.setCommandBufferCount(1);
	commandAllocInfo.setLevel(CommandBufferLevel::ePrimary);

	Device device = Engine::GetRendererInstance()->GetDevice();
	CommandBuffer commandBuffer = device.allocateCommandBuffers(commandAllocInfo)[0];

	CommandBufferBeginInfo commandBeginInfo;
	commandBeginInfo.setFlags(CommandBufferUsageFlagBits::eOneTimeSubmit);

	BufferCopy copyRegion;
	copyRegion.setSize(std::min<uint32_t>(inSrc.GetSize(), inDst.GetSize()));
	copyRegion.setSrcOffset(0);
	copyRegion.setDstOffset(0);

	commandBuffer.begin(commandBeginInfo);
	commandBuffer.copyBuffer(inSrc.GetBuffer(), inDst.GetBuffer(), 1, &copyRegion);
	commandBuffer.end();

	SubmitInfo submitInfo;
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&commandBuffer);

	Engine::GetRendererInstance()->GetGraphicsQueue().submit({ submitInfo }, Fence());
	Engine::GetRendererInstance()->GetGraphicsQueue().waitIdle();

	device.freeCommandBuffers(Engine::GetRendererInstance()->GetCommandPool(), { commandBuffer });
}

uint32_t MemoryBuffer::FindMemoryType(uint32_t inTypeFilter, MemoryPropertyFlags inPropFlags)
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

