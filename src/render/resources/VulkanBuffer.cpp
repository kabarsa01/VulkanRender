#include "VulkanBuffer.h"
#include "core/Engine.h"

VulkanBuffer::VulkanBuffer(bool inScoped)
	: scoped(inScoped)
{

}

VulkanBuffer::~VulkanBuffer()
{
	if (scoped)
	{
		Destroy();
	}
}

void VulkanBuffer::Create(VulkanDevice* inDevice)
{
	if (buffer)
	{
		return;
	}
	vulkanDevice = inDevice;
	buffer = vulkanDevice->GetDevice().createBuffer(createInfo);
}

void VulkanBuffer::Destroy()
{
	if (buffer)
	{
		vulkanDevice->GetDevice().destroyBuffer(buffer);
		buffer = nullptr;
		DeviceMemoryManager::GetInstance()->ReturnMemory(memRecord);
	}
}

void VulkanBuffer::BindMemory(MemoryPropertyFlags inMemPropertyFlags)
{
	DeviceMemoryManager* dmm = DeviceMemoryManager::GetInstance();
	memRecord = dmm->RequestMemory(GetMemoryRequirements(), inMemPropertyFlags);
	vulkanDevice->GetDevice().bindBufferMemory(buffer, memRecord.pos.memory, memRecord.pos.offset);
}

void VulkanBuffer::BindMemory(const DeviceMemory& inDeviceMemory, DeviceSize inMemOffset)
{
	vulkanDevice->GetDevice().bindBufferMemory(buffer, inDeviceMemory, inMemOffset);
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
	return vulkanDevice->GetDevice().getBufferMemoryRequirements(buffer);
}

MemoryRecord& VulkanBuffer::GetMemoryRecord()
{
	return memRecord;
}

void VulkanBuffer::SubmitCopyCommand(const VulkanBuffer& inSrc, const VulkanBuffer& inDst)
{
	CommandBuffer commandBuffer = Engine::GetRendererInstance()->GetCommandBuffers().GetNextTransferBuffer();

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

	Queue& transferQueue = Engine::GetRendererInstance()->GetVulkanDevice().GetTransferQueue();
	transferQueue.submit({ submitInfo }, Fence());
	transferQueue.waitIdle();
}

