#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "data/Resource.h"
#include "vulkan/vulkan.hpp"
#include "render/resources/VulkanBuffer.h"
#include "render/resources/VulkanDeviceMemory.h"

using namespace std;
using namespace glm;
using namespace VULKAN_HPP_NAMESPACE;

//packing should probably be considered, maybe in the future someday
//#pragma pack(push, 1)
//#pragma pack(pop)

struct Vertex
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoord;
	vec3 Tangent;
	vec3 Bitangent;

	static std::array<VertexInputAttributeDescription, 5> GetAttributeDescriptions();
};


class MeshData : public Resource
{
public:
	vector<Vertex> vertices;
	vector<uint32_t> indices;

	MeshData(const string& InId);
	MeshData(const string& InId, const std::vector<Vertex>& inVertices, const std::vector<unsigned int>& inIndices);
	virtual ~MeshData();

	virtual void OnDestroy() override;

	// Inherited via Resource
	virtual bool Load() override;
	virtual bool Unload() override;

	void CreateBuffer();
	void DestroyBuffer();
	void Draw();

	Buffer GetVertexBuffer();
	uint32_t GetVertexBufferSizeBytes();
	uint32_t GetVertexCount();
	Buffer GetIndexBuffer();
	uint32_t GetIndexBufferSizeBytes();
	uint32_t GetIndexCount();

	static VertexInputBindingDescription GetBindingDescription();
	// fullscreen quad instance to be used for screen space stuff
	static std::shared_ptr<MeshData> FullscreenQuad();
private:
	VulkanBuffer vertexBuffer;
	VulkanBuffer indexBuffer;

	MeshData() : Resource(HashString::NONE()) {}

	void CreateBuffer(
		VulkanBuffer& inBuffer,
		DeviceSize inSize, BufferUsageFlags usage, 
		SharingMode inSharingMode, MemoryPropertyFlags inMemPropFlags);
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
	DeviceSize size = static_cast<DeviceSize>(sizeof(T) * inDataVector.size());

	VulkanBuffer stagingBuffer;
	VulkanDeviceMemory stagingMemory;
	CreateBuffer(stagingBuffer, size, BufferUsageFlagBits::eTransferSrc, SharingMode::eExclusive, MemoryPropertyFlagBits::eHostCoherent | MemoryPropertyFlagBits::eHostVisible);
	MemoryRecord& memRec = stagingBuffer.GetMemoryRecord();
	memRec.pos.memory.MapCopyUnmap(MemoryMapFlags(), memRec.pos.offset, size, inDataVector.data(), 0, size);

	CreateBuffer(inBuffer, size, usage | BufferUsageFlagBits::eTransferDst,	SharingMode::eExclusive, MemoryPropertyFlagBits::eDeviceLocal);

	VulkanBuffer::SubmitCopyCommand(stagingBuffer, inBuffer);
}



