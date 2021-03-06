#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "data/Resource.h"
#include "vulkan/vulkan.hpp"
#include "render/resources/VulkanBuffer.h"
#include "render/resources/VulkanDeviceMemory.h"
#include "core/Engine.h"
#include "render/Renderer.h"

namespace CGE
{
	using VULKAN_HPP_NAMESPACE::VertexInputAttributeDescription;
	using VULKAN_HPP_NAMESPACE::VertexInputBindingDescription;
	using VULKAN_HPP_NAMESPACE::BufferUsageFlags;
	
	//packing should probably be considered, maybe in the future someday
	//#pragma pack(push, 1)
	//#pragma pack(pop)
	
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoord;
		glm::vec3 tangent;
		glm::vec3 bitangent;
	
		static std::array<VertexInputAttributeDescription, 5> GetAttributeDescriptions(uint32_t inDesiredBinding);
	};
	
	
	class MeshData : public Resource
	{
	public:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	
		MeshData(const HashString& InId);
		MeshData(const HashString& InId, const std::vector<Vertex>& inVertices, const std::vector<unsigned int>& inIndices);
		virtual ~MeshData();
	
		virtual void OnDestroy() override;
	
		// Inherited via Resource
		virtual bool Create() override;
	
		void CreateBuffer();
		void DestroyBuffer();
		void Draw();
	
		VulkanBuffer& GetVertexBuffer();
		uint32_t GetVertexBufferSizeBytes();
		uint32_t GetVertexCount();
		VulkanBuffer& GetIndexBuffer();
		uint32_t GetIndexBufferSizeBytes();
		uint32_t GetIndexCount();
	
		static VertexInputBindingDescription GetBindingDescription(uint32_t inDesiredBinding);
		// fullscreen quad instance to be used for screen space stuff
		static std::shared_ptr<MeshData> FullscreenQuad();
	protected:
		virtual bool Destroy() override;
	private:
		static std::shared_ptr<MeshData> m_fullscreenQuad;

		VulkanBuffer vertexBuffer;
		VulkanBuffer indexBuffer;
	
		MeshData() : Resource(HashString::NONE) {}
	
		template<class T>
		void SetupBuffer(VulkanBuffer& inBuffer, std::vector<T>& inDataVector, BufferUsageFlags usage);
	};
	
	typedef std::shared_ptr<MeshData> MeshDataPtr;
	
	//--------------------------------------------------------------------------------------------------------------------------
	// template defs
	//--------------------------------------------------------------------------------------------------------------------------
	
	template<class T>
	void MeshData::SetupBuffer(VulkanBuffer& inBuffer, std::vector<T>& inDataVector, BufferUsageFlags usage)
	{
		if (indexBuffer)
		{
			return;
		}
	
		DeviceSize size = static_cast<DeviceSize>(sizeof(T) * inDataVector.size());
	
		inBuffer.createInfo.setSize(size);
		inBuffer.createInfo.setUsage(usage | BufferUsageFlagBits::eTransferDst | BufferUsageFlagBits::eShaderDeviceAddress);
		inBuffer.createInfo.setSharingMode(SharingMode::eExclusive);
		inBuffer.Create(&Engine::GetRendererInstance()->GetVulkanDevice());
		inBuffer.BindMemory(MemoryPropertyFlagBits::eDeviceLocal);
		inBuffer.SetData(size, reinterpret_cast<char*>( inDataVector.data() ));
	
	//	VulkanBuffer::SubmitCopyCommand(*inBuffer.CreateStagingBuffer(), inBuffer);
	}
	
	
	
}
