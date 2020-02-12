#include "MeshData.h"
#include "data/DataManager.h"

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

MeshData::MeshData(const string& InId)
	: Resource{InId}
{

}

MeshData::MeshData(const string& InId, const std::vector<Vertex>& InVertices, const std::vector<unsigned int>& InIndices)
	: Resource{ InId }
	, Vertices( InVertices )
	, Indices( InIndices )
{
}

MeshData::~MeshData()
{

}

void MeshData::OnDestroy()
{
	DestroyBufferObjects();
}

void MeshData::SetupBufferObjects()
{
	// VAO & VBO init
}

void MeshData::DestroyBufferObjects()
{
}

void MeshData::Draw()
{
	// bind VAO and draw
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
