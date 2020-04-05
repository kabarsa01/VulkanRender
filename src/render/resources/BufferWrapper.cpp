#include "BufferWrapper.h"
#include "core/Engine.h"

BufferWrapper::BufferWrapper()
{

}

BufferWrapper::~BufferWrapper()
{
	Destroy();
}

void BufferWrapper::Create()
{
	if (buffer)
	{
		return;
	}
	device = Engine::GetRendererInstance()->GetDevice();
	buffer = device.createBuffer(createInfo);
}

void BufferWrapper::Destroy()
{
	if (buffer)
	{
		device.destroyBuffer(buffer);
		buffer = nullptr;
		DeviceMemoryManager::GetInstance()->ReturnMemory(memRecord);
	}
}

void BufferWrapper::BindMemory(MemoryPropertyFlags inMemPropertyFlags)
{
	DeviceMemoryManager* dmm = DeviceMemoryManager::GetInstance();
	memRecord = dmm->RequestMemory(GetMemoryRequirements(), inMemPropertyFlags);
	device.bindBufferMemory(buffer, memRecord.memory, memRecord.memoryOffset);
}

void BufferWrapper::BindMemory(const DeviceMemory& inDeviceMemory, DeviceSize inMemOffset)
{
	device.bindBufferMemory(buffer, inDeviceMemory, inMemOffset);
}

Buffer& BufferWrapper::GetBuffer()
{
	return buffer;
}

Buffer BufferWrapper::GetBuffer() const
{
	return buffer;
}

MemoryRequirements BufferWrapper::GetMemoryRequirements()
{
	return device.getBufferMemoryRequirements(buffer);
}

MemoryRecord& BufferWrapper::GetMemoryRecord()
{
	return memRecord;
}

void BufferWrapper::SubmitCopyCommand(const BufferWrapper& inSrc, const BufferWrapper& inDst)
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

