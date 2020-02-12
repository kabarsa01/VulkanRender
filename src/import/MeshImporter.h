#pragma once

#include <string>
#include <vector>
#include <memory>
#include "render/MeshData.h"

#include <assimp/scene.h>

//struct aiNode;
//struct aiScene;

class MeshImporter
{
public:
	void Import(std::string InPath);
	std::vector<std::shared_ptr<MeshData>>& GetMeshes();
protected:
	std::string Path;
	std::vector<std::shared_ptr<MeshData>> Meshes;

	void ProcessNode(aiNode *Node, const aiScene *Scene);
	std::shared_ptr<MeshData> ProcessMesh(aiMesh* AiMesh);
};

