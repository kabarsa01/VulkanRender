#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "data/Resource.h"
#include "vulkan/vulkan.hpp"
#include "render/MemoryBuffer.h"

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

	VertexInputBindingDescription GetBindingDescription();
	Buffer GetVertexBuffer();
	uint32_t GetVertexBufferSizeBytes();
	uint32_t GetVertexCount();
	Buffer GetIndexBuffer();
	uint32_t GetIndexBufferSizeBytes();
	uint32_t GetIndexCount();
	// fullscreen quad instance to be used for screen space stuff
	static std::shared_ptr<MeshData> FullscreenQuad();
private:
	MemoryBuffer vertexBuffer;
	MemoryBuffer indexBuffer;
	//VULKAN_HPP_NAMESPACE::Buffer vertexBuffer;
	//VULKAN_HPP_NAMESPACE::DeviceMemory vertexBufferMemory;

	MeshData() : Resource(HashString::NONE()) {}
	uint32_t FindMemoryType(uint32_t inTypeFilter, VULKAN_HPP_NAMESPACE::MemoryPropertyFlags inPropFlags);
	template<class T>
	static void SetupBuffer(MemoryBuffer& memBuffer, std::vector<T>& inDataVector, BufferUsageFlags usage);
};

typedef std::shared_ptr<MeshData> MeshDataPtr;

//--------------------------------------------------------------------------------------------------------------------------
// template defs
//--------------------------------------------------------------------------------------------------------------------------

template<class T>
void MeshData::SetupBuffer(MemoryBuffer& memBuffer, std::vector<T>& inDataVector, BufferUsageFlags usage)
{
	MemoryBuffer stagingBuffer;
	stagingBuffer.SetSize(static_cast<uint32_t>(sizeof(T) * inDataVector.size()));
	stagingBuffer.SetUsage(BufferUsageFlagBits::eTransferSrc);
	stagingBuffer.SetMemProperty(MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent);
	stagingBuffer.Create();

	stagingBuffer.CopyData(inDataVector.data(), MemoryMapFlags(), 0);

	memBuffer.SetSize(stagingBuffer.GetSize());
	memBuffer.SetUsage(usage | BufferUsageFlagBits::eTransferDst);
	memBuffer.SetMemProperty(MemoryPropertyFlagBits::eDeviceLocal);
	memBuffer.Create();

	MemoryBuffer::CopyBuffer(stagingBuffer, memBuffer);
}



