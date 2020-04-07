#pragma once

#include <string>
#include <vector>
#include <memory>

#include <assimp/scene.h>
#include "data/MeshData.h"

//struct aiNode;
//struct aiScene;

class MeshImporter
{
public:
	void Import(std::string inPath);
	std::vector<std::shared_ptr<MeshData>>& GetMeshes();
protected:
	std::string path;
	std::vector<std::shared_ptr<MeshData>> meshes;

	void ProcessNode(aiNode *inNode, const aiScene *inScene);
	std::shared_ptr<MeshData> ProcessMesh(aiMesh* inAiMesh);
};

