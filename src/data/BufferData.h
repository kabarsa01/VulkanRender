#ifndef __BUFFER_DATA_H__
#define __BUFFER_DATA_H__

#include <memory>

#include "Resource.h"
#include "common/HashString.h"
#include "render/resources/VulkanBuffer.h"
#include "vulkan/vulkan.hpp"

namespace CGE
{

	// Basically a wrapper around VulkanBuffer to have it as a Resource to use TextureData and BufferData
	// resources in DataManager, RenderPassResourceTable and other resource tracking systems. Why not make
	// VulkanBuffer a Resource? BufferData might be a better option to wrap all memory binding, staging
	// buffer creation etc, the same way TextureData wraps VulkanImage
	class BufferData : public Resource
	{
	public:
		BufferData(HashString id, vk::DeviceSize size, vk::BufferUsageFlags usage, bool deviceLocal = true);
		BufferData(HashString id, vk::BufferCreateInfo createInfo, bool deviceLocal = true);
		~BufferData() {}

		bool Create() override;
		void CopyTo(/*vk::DeviceSize offset, */vk::DeviceSize size, const char* data);
	protected:
		bool Destroy() override;
	private:
		VulkanBuffer m_buffer;

		vk::DeviceSize m_size;
		vk::BufferUsageFlags m_usage;
		bool m_deviceLocal;
		bool m_externalCreateInfo;
	};

	typedef std::shared_ptr<BufferData> BufferDataPtr;

}

#endif

