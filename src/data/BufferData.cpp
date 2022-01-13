#include "./BufferData.h"
#include "core/Engine.h"
#include "render/Renderer.h"
#include "render/TransferList.h"

namespace CGE
{

	BufferData::BufferData(HashString id, vk::DeviceSize size, vk::BufferUsageFlags usage, bool deviceLocal/*= true*/)
		: Resource(id)
		, m_size(size)
		, m_usage(usage)
		, m_deviceLocal(deviceLocal)
		, m_externalCreateInfo(false)
		, m_cleanup(true)
	{
	}

	BufferData::BufferData(HashString id, vk::BufferCreateInfo createInfo, bool deviceLocal/*= true*/)
		: Resource(id)
		, m_size(createInfo.size)
		, m_usage(createInfo.usage)
		, m_deviceLocal(deviceLocal)
		, m_externalCreateInfo(true)
		, m_cleanup(true)
	{
		m_buffer.createInfo = createInfo;
	}

	BufferData::BufferData(HashString id, const VulkanBuffer& buffer, bool cleanup /*= false*/)
		: Resource(id)
		, m_size(buffer.createInfo.size)
		, m_usage(buffer.createInfo.usage)
		, m_deviceLocal(buffer.GetMemoryRecord().deviceLocal)
		, m_externalCreateInfo(true)
		, m_cleanup(cleanup)
	{
		m_buffer = buffer;
	}

	BufferData::~BufferData()
	{
		if (m_cleanup)
		{
			Destroy();
		}
	}

	bool BufferData::Create()
	{
		if (m_buffer)
		{
			return false;
		}

		if (!m_externalCreateInfo)
		{
			m_buffer.createInfo.setSharingMode(vk::SharingMode::eExclusive);
			m_buffer.createInfo.setSize(m_size);
			m_buffer.createInfo.setUsage(m_usage | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress);
		}

		m_buffer.Create(m_deviceLocal);

		return true;
	}

	void BufferData::CopyTo(/*vk::DeviceSize offset, */vk::DeviceSize size, const char* data)
	{
		if (data == nullptr)
		{
			return;
		}

		if (m_buffer.IsDeviceLocal())
		{
			if (!m_staging)
			{
				m_staging = CreateStaging(size);
			}
			m_staging->CopyTo(size, data);
			TransferList::GetInstance()->PushBuffer(get_shared_from_this<BufferData>());
		}
		else
		{
			m_buffer.CopyTo(size, data);
		}
	}

	bool BufferData::Destroy()
	{
		m_buffer.Destroy();
		return true;
	}

	std::shared_ptr<BufferData> BufferData::CreateStaging(vk::DeviceSize size)
	{
		vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eTransferSrc | m_buffer.createInfo.usage;
		BufferDataPtr buffer = ObjectBase::NewObject<BufferData>(GetResourceId() + HashString("_staging"), size, usage, false);
		buffer->Create();
		return buffer;
	}

}

