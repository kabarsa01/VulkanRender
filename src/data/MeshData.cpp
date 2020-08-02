#include "MeshData.h"
#include "data/DataManager.h"
#include "core/Engine.h"

using namespace VULKAN_HPP_NAMESPACE;

//-------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------

namespace
{
	std::vector<Vertex> quadVertices = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	// positions            // normals           // texCoords
	{{-1.0f,  1.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
	{{-1.0f, -1.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
	{{ 1.0f, -1.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
	{{ 1.0f,  1.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}}
	};

	std::vector<unsigned int> quadIndices = {
		0, 1, 2,
		0, 2, 3
	};

	std::string fullscreenQuadId = "MeshData_FullscreenQuad";
};

//-------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------
//////////////////
//vec3 Position;
//vec3 Normal;
//vec2 TexCoord;
//vec3 Tangent;
//vec3 Bitangent;
/////////////////
std::array<VertexInputAttributeDescription, 5> Vertex::GetAttributeDescriptions(uint32_t inDesiredBinding)
{
	std::array<VertexInputAttributeDescription, 5> attributes = {};

	// vec3 Position
	attributes[0].setBinding(inDesiredBinding);
	attributes[0].setLocation(0);
	attributes[0].setFormat(Format::eR32G32B32Sfloat);
	attributes[0].setOffset(offsetof(Vertex, position));

	// vec3 Normal
	attributes[1].setBinding(inDesiredBinding);
	attributes[1].setLocation(1);
	attributes[1].setFormat(Format::eR32G32B32Sfloat);
	attributes[1].setOffset(offsetof(Vertex, normal));

	// vec2 TexCoord
	attributes[2].setBinding(inDesiredBinding);
	attributes[2].setLocation(2);
	attributes[2].setFormat(Format::eR32G32Sfloat);
	attributes[2].setOffset(offsetof(Vertex, texCoord));

	// vec3 Tangent
	attributes[3].setBinding(inDesiredBinding);
	attributes[3].setLocation(3);
	attributes[3].setFormat(Format::eR32G32B32Sfloat);
	attributes[3].setOffset(offsetof(Vertex, tangent));

	// vec3 BiTangent
	attributes[4].setBinding(inDesiredBinding);
	attributes[4].setLocation(4);
	attributes[4].setFormat(Format::eR32G32B32Sfloat);
	attributes[4].setOffset(offsetof(Vertex, bitangent));

	return attributes;
}

//-------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------

MeshData::MeshData(const HashString& inId)
	: Resource{inId}
{

}

MeshData::MeshData(const HashString& inId, const std::vector<Vertex>& inVertices, const std::vector<unsigned int>& inIndices)
	: Resource{ inId }
	, vertices( inVertices )
	, indices( inIndices )
{
}

MeshData::~MeshData()
{
}

void MeshData::OnDestroy()
{
	DestroyBuffer();
}

void MeshData::CreateBuffer()
{
	SetupBuffer<Vertex>(vertexBuffer, vertices, BufferUsageFlagBits::eVertexBuffer);
	SetupBuffer<uint32_t>(indexBuffer, indices, BufferUsageFlagBits::eIndexBuffer);
}

void MeshData::DestroyBuffer()
{
	// buffers
	vertexBuffer.Destroy();
	indexBuffer.Destroy();
}

void MeshData::Draw()
{
	// bind VAO and draw
}

VertexInputBindingDescription MeshData::GetBindingDescription(uint32_t inDesiredBinding)
{
	VertexInputBindingDescription bindingDescription;

	bindingDescription.setBinding(inDesiredBinding);
	bindingDescription.setStride(sizeof(Vertex));
	bindingDescription.setInputRate(VertexInputRate::eVertex);

	return bindingDescription;
}

VulkanBuffer& MeshData::GetVertexBuffer()
{
	return vertexBuffer;
}

uint32_t MeshData::GetVertexBufferSizeBytes()
{
	return static_cast<uint32_t>( sizeof(Vertex) * vertices.size() );
}

uint32_t MeshData::GetVertexCount()
{
	return static_cast<uint32_t>( vertices.size() );
}

VulkanBuffer& MeshData::GetIndexBuffer()
{
	return indexBuffer;
}

uint32_t MeshData::GetIndexBufferSizeBytes()
{
	return static_cast<uint32_t>(sizeof(uint32_t) * indices.size());
}

uint32_t MeshData::GetIndexCount()
{
	return static_cast<uint32_t>(indices.size());
}

MeshDataPtr MeshData::FullscreenQuad()
{
	return DataManager::GetInstance()->RequestResourceByType<MeshData, const std::vector<Vertex>&, const std::vector<unsigned int>&>(
		fullscreenQuadId,
		quadVertices,
		quadIndices
	);
}

bool MeshData::Load()
{
	return true;
}

bool MeshData::Cleanup()
{
	DestroyBuffer();
	return true;
}


