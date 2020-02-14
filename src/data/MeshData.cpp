#include "MeshData.h"
#include "data/DataManager.h"
#include "core/Engine.h"

using namespace VULKAN_HPP_NAMESPACE;

//-------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------

namespace
{
	std::vector<Vertex> QuadVertices = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions          // texCoords
	{{-1.0f,  1.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
	{{-1.0f, -1.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
	{{ 1.0f, -1.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
	{{ 1.0f,  1.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}}
	};

	std::vector<unsigned int> QuadIndices = {
		0, 1, 2,
		0, 2, 3
	};

	std::string FullscreenQuadId = "MeshData_FullscreenQuad";
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
std::array<VertexInputAttributeDescription, 5> Vertex::GetAttributeDescriptions()
{
	std::array<VertexInputAttributeDescription, 5> attributes = {};

	// vec3 Position
	attributes[0].setBinding(0);
	attributes[0].setLocation(0);
	attributes[0].setFormat(Format::eR32G32B32Sfloat);
	attributes[0].setOffset(offsetof(Vertex, Position));

	// vec3 Normal
	attributes[1].setBinding(0);
	attributes[1].setLocation(1);
	attributes[1].setFormat(Format::eR32G32B32Sfloat);
	attributes[1].setOffset(offsetof(Vertex, Normal));

	// vec2 TexCoord
	attributes[2].setBinding(0);
	attributes[2].setLocation(2);
	attributes[2].setFormat(Format::eR32G32Sfloat);
	attributes[2].setOffset(offsetof(Vertex, TexCoord));

	// vec3 Tangent
	attributes[3].setBinding(0);
	attributes[3].setLocation(3);
	attributes[3].setFormat(Format::eR32G32B32Sfloat);
	attributes[3].setOffset(offsetof(Vertex, Tangent));

	// vec3 BiTangent
	attributes[4].setBinding(0);
	attributes[4].setLocation(4);
	attributes[4].setFormat(Format::eR32G32B32Sfloat);
	attributes[4].setOffset(offsetof(Vertex, Bitangent));

	return attributes;
}

//-------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------

MeshData::MeshData(const string& inId)
	: Resource{inId}
{

}

MeshData::MeshData(const string& inId, const std::vector<Vertex>& inVertices, const std::vector<unsigned int>& inIndices)
	: Resource{ inId }
	, vertices( inVertices )
	, indices( inIndices )
{
}

uint32_t MeshData::FindMemoryType(uint32_t inTypeFilter, VULKAN_HPP_NAMESPACE::MemoryPropertyFlags inPropFlags)
{
	PhysicalDeviceMemoryProperties memProps;
	memProps = Engine::GetRendererInstance()->GetPhysicalDevice().getMemoryProperties();

	for (uint32_t index = 0; index < memProps.memoryTypeCount; index++)
	{
		bool propFlagsSufficient = (memProps.memoryTypes[index].propertyFlags & inPropFlags) == inPropFlags;
		bool hasTheType = inTypeFilter & (1 << index);
		if (hasTheType && propFlagsSufficient)
		{
			return index;
		}
	}

	throw std::runtime_error("No suitable memory type found");
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
	MemoryBuffer stagingBuffer;
	stagingBuffer.SetSize(static_cast<uint32_t>( sizeof(Vertex) * vertices.size() ));
	stagingBuffer.SetUsage(BufferUsageFlagBits::eTransferSrc);
	stagingBuffer.SetMemProperty(MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent);
	stagingBuffer.Create();

	stagingBuffer.CopyData(vertices.data(), MemoryMapFlags(), 0);

	vertexBuffer.SetSize(stagingBuffer.GetSize());
	vertexBuffer.SetUsage(BufferUsageFlagBits::eVertexBuffer | BufferUsageFlagBits::eTransferDst);
	vertexBuffer.SetMemProperty(MemoryPropertyFlagBits::eDeviceLocal);
	vertexBuffer.Create();

	MemoryBuffer::CopyBuffer(stagingBuffer, vertexBuffer);
//	stagingBuffer.Destroy();
}

void MeshData::DestroyBuffer()
{
	vertexBuffer.Destroy();
}

void MeshData::Draw()
{
	// bind VAO and draw
}

VULKAN_HPP_NAMESPACE::VertexInputBindingDescription MeshData::GetBindingDescription()
{
	VertexInputBindingDescription bindingDescription;

	bindingDescription.setBinding(0);
	bindingDescription.setStride(sizeof(Vertex));
	bindingDescription.setInputRate(VertexInputRate::eVertex);

	return bindingDescription;
}

VULKAN_HPP_NAMESPACE::Buffer MeshData::GetVertexBuffer()
{
	return vertexBuffer.GetBuffer();
}

uint32_t MeshData::GetSizeBytes()
{
	return static_cast<uint32_t>( sizeof(Vertex) * vertices.size() );
}

uint32_t MeshData::GetVertexCount()
{
	return static_cast<uint32_t>( vertices.size() );
}

MeshDataPtr MeshData::FullscreenQuad()
{
	return DataManager::GetInstance()->RequestResourceByType<MeshData, const string&, const std::vector<Vertex>&, const std::vector<unsigned int>&>(
		FullscreenQuadId,
		FullscreenQuadId,
		QuadVertices,
		QuadIndices
	);
}

bool MeshData::Load()
{
	return false;
}

bool MeshData::Unload()
{
	return false;
}


