#include "VulkanBuffer.h"
#include "core/Engine.h"

VulkanBuffer::VulkanBuffer()
{

}

VulkanBuffer::~VulkanBuffer()
{
	Destroy();
}

void VulkanBuffer::Create()
{
	if (buffer)
	{
		return;
	}
	device = Engine::GetRendererInstance()->GetVulkanDevice();
	buffer = device.createBuffer(createInfo);
}

void VulkanBuffer::Destroy()
{
	if (buffer)
	{
		device.destroyBuffer(buffer);
		buffer = nullptr;
		DeviceMemoryManager::GetInstance()->ReturnMemory(memRecord);
	}
}

void VulkanBuffer::BindMemory(MemoryPropertyFlags inMemPropertyFlags)
{
	DeviceMemoryManager* dmm = DeviceMemoryManager::GetInstance();
	memRecord = dmm->RequestMemory(GetMemoryRequirements(), inMemPropertyFlags);
	device.bindBufferMemory(buffer, memRecord.pos.memory, memRecord.pos.offset);
}

void VulkanBuffer::BindMemory(const DeviceMemory& inDeviceMemory, DeviceSize inMemOffset)
{
	device.bindBufferMemory(buffer, inDeviceMemory, inMemOffset);
}

Buffer& VulkanBuffer::GetBuffer()
{
	return buffer;
}

Buffer VulkanBuffer::GetBuffer() const
{
	return buffer;
}

MemoryRequirements VulkanBuffer::GetMemoryRequirements()
{
	return device.getBufferMemoryRequirements(buffer);
}

MemoryRecord& VulkanBuffer::GetMemoryRecord()
{
	return memRecord;
}

void VulkanBuffer::SubmitCopyCommand(const VulkanBuffer& inSrc, const VulkanBuffer& inDst)
{
	CommandBufferAllocateInfo commandAllocInfo;
	commandAllocInfo.setCommandPool(Engine::GetRendererInstance()->GetCommandPool());
	commandAllocInfo.setCommandBufferCount(1);
	commandAllocInfo.setLevel(CommandBufferLevel::ePrimary);

	Device device = Engine::GetRendererInstance()->GetVulkanDevice();
	CommandBuffer commandBuffer = device.allocateCommandBuffers(commandAllocInfo)[0];

	CommandBufferBeginInfo commandBeginInfo;
	commandBeginInfo.setFlags(CommandBufferUsageFlagBits::eOneTimeSubmit);

	BufferCopy copyRegion;
	copyRegion.setSize(std::min<DeviceSize>(inSrc.createInfo.size, inDst.createInfo.size));
	copyRegion.setSrcOffset(0);
	copyRegion.setDstOffset(0);

	commandBuffer.begin(commandBeginInfo);
	commandBuffer.copyBuffer(inSrc, inDst, 1, &copyRegion);
	commandBuffer.end();

	SubmitInfo submitInfo;
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&commandBuffer);

	Engine::GetRendererInstance()->GetGraphicsQueue().submit({ submitInfo }, Fence());
	Engine::GetRendererInstance()->GetGraphicsQueue().waitIdle();

	device.freeCommandBuffers(Engine::GetRendererInstance()->GetCommandPool(), { commandBuffer });
}

