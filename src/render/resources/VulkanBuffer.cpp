#include "VulkanBuffer.h"
#include "core/Engine.h"

VulkanBuffer::VulkanBuffer(bool inScoped)
	: scoped(inScoped)
	, stagingBuffer(nullptr)
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

void VulkanBuffer::SetData(const std::vector<char>& inData)
{
	data = inData;
}

void VulkanBuffer::SetData(DeviceSize inSize, char* inData)
{
	data.assign(inData, inData + inSize);
}

void VulkanBuffer::Destroy()
{
	if (stagingBuffer != nullptr)
	{
		stagingBuffer->Destroy();
		delete stagingBuffer;
		stagingBuffer = nullptr;
	}
	if (buffer)
	{
		vulkanDevice->GetDevice().destroyBuffer(buffer);
		buffer = nullptr;
		DeviceMemoryManager::GetInstance()->ReturnMemory(memRecord);
	}
}

void VulkanBuffer::BindMemory(MemoryPropertyFlags inMemPropertyFlags)
{
	if (memRecord.pos.valid)
	{
		return;
	}
	DeviceMemoryManager* dmm = DeviceMemoryManager::GetInstance();
	memRecord = dmm->RequestMemory(GetMemoryRequirements(), inMemPropertyFlags);
	vulkanDevice->GetDevice().bindBufferMemory(buffer, memRecord.pos.memory, memRecord.pos.offset);
}

void VulkanBuffer::BindMemory(const DeviceMemory& inDeviceMemory, DeviceSize inMemOffset)
{
	vulkanDevice->GetDevice().bindBufferMemory(buffer, inDeviceMemory, inMemOffset);
}

VulkanBuffer* VulkanBuffer::CreateStagingBuffer()
{
	return CreateStagingBuffer(createInfo.size, data.data());
}

VulkanBuffer* VulkanBuffer::CreateStagingBuffer(DeviceSize inSize, char* inData)
{
	if (stagingBuffer)
	{
		return stagingBuffer;
	}

	stagingBuffer = new VulkanBuffer();
	stagingBuffer->createInfo.setSize(inSize);
	stagingBuffer->createInfo.setUsage(BufferUsageFlagBits::eTransferSrc);
	stagingBuffer->createInfo.setSharingMode(SharingMode::eExclusive);
	stagingBuffer->Create(vulkanDevice);
	stagingBuffer->BindMemory(MemoryPropertyFlagBits::eHostCoherent | MemoryPropertyFlagBits::eHostVisible);
	MemoryRecord& memRec = stagingBuffer->GetMemoryRecord();
	memRec.pos.memory.MapCopyUnmap(MemoryMapFlags(), memRec.pos.offset, inSize, inData, 0, inSize);

	return stagingBuffer;
}

BufferCopy VulkanBuffer::CreateBufferCopy()
{
	BufferCopy copyRegion;
	copyRegion.setSize(createInfo.size);
	copyRegion.setSrcOffset(0);
	copyRegion.setDstOffset(0);

	return copyRegion;
}

BufferMemoryBarrier VulkanBuffer::CreateMemoryBarrier(uint32_t inSrcQueue, uint32_t inDstQueue, AccessFlags inSrcAccessMask, AccessFlags inDstAccessMask)
{
	BufferMemoryBarrier barrier;

	barrier.setBuffer(buffer);
	barrier.setSize(createInfo.size);
	barrier.setOffset(0);
	barrier.setSrcQueueFamilyIndex(inSrcQueue);
	barrier.setDstQueueFamilyIndex(inDstQueue);
	barrier.setSrcAccessMask(inSrcAccessMask);
	barrier.setDstAccessMask(inDstAccessMask);

	return barrier;
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
//
//void VulkanBuffer::SubmitCopyCommand(const VulkanBuffer& inSrc, const VulkanBuffer& inDst)
//{
//	CommandBuffer commandBuffer = Engine::GetRendererInstance()->GetCommandBuffers().GetNextTransferBuffer();
//
//	CommandBufferBeginInfo commandBeginInfo;
//	commandBeginInfo.setFlags(CommandBufferUsageFlagBits::eOneTimeSubmit);
//
//	BufferCopy copyRegion;
//	copyRegion.setSize(std::min<DeviceSize>(inSrc.createInfo.size, inDst.createInfo.size));
//	copyRegion.setSrcOffset(0);
//	copyRegion.setDstOffset(0);
//
//	commandBuffer.begin(commandBeginInfo);
//	commandBuffer.copyBuffer(inSrc, inDst, 1, &copyRegion);
//	commandBuffer.end();
//
//	SubmitInfo submitInfo;
//	submitInfo.setCommandBufferCount(1);
//	submitInfo.setPCommandBuffers(&commandBuffer);
//
//	Queue& transferQueue = Engine::GetRendererInstance()->GetVulkanDevice().GetTransferQueue();
//	transferQueue.submit({ submitInfo }, Fence());
//	transferQueue.waitIdle();
//}

