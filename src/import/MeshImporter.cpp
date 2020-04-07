#include "import/MeshImporter.h"
#include "core/ObjectBase.h"

#include <iostream>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>


void MeshImporter::Import(std::string inPath)
{
	path = inPath;
	Assimp::Importer LocalImporter;
	const aiScene* Scene = LocalImporter.ReadFile(inPath, aiProcess_Triangulate | aiProcess_CalcTangentSpace);// | aiProcess_FlipUVs);

	if ( (Scene == nullptr) || (Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || (Scene->mRootNode == nullptr) )
	{
		std::cout << "ASSIMP::ERROR import " << LocalImporter.GetErrorString() << std::endl;

		if (Scene != nullptr)
		{
//			delete Scene;
		}
		return;
	}

	ProcessNode(Scene->mRootNode, Scene);
}

std::vector<std::shared_ptr<MeshData>>& MeshImporter::GetMeshes()
{
	return meshes;
}

void MeshImporter::ProcessNode(aiNode *inNode, const aiScene *inScene)
{
	for (unsigned int MeshIndex = 0; MeshIndex < inNode->mNumMeshes; MeshIndex++)
	{
		aiMesh* Mesh = inScene->mMeshes[inNode->mMeshes[MeshIndex]];
		meshes.push_back(ProcessMesh(Mesh));
	}

	// recursive children processing
	for (unsigned int ChildIndex = 0; ChildIndex < inNode->mNumChildren; ChildIndex++)
	{
		ProcessNode(inNode->mChildren[ChildIndex], inScene);
	}
}

std::shared_ptr<MeshData> MeshImporter::ProcessMesh(aiMesh* inAiMesh)
{
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;

	for (unsigned int Index = 0; Index < inAiMesh->mNumVertices; Index++)
	{
		Vertex V;

		V.Position.x = inAiMesh->mVertices[Index].x;
		V.Position.y = inAiMesh->mVertices[Index].y;
		V.Position.z = inAiMesh->mVertices[Index].z;

		V.Normal.x = inAiMesh->mNormals[Index].x;
		V.Normal.y = inAiMesh->mNormals[Index].y;
		V.Normal.z = inAiMesh->mNormals[Index].z;

		// we use only one set of texture coordinates for one
		if (inAiMesh->HasTextureCoords(0))
		{
			V.TexCoord.x = inAiMesh->mTextureCoords[0][Index].x;
			V.TexCoord.y = inAiMesh->mTextureCoords[0][Index].y;
		}
		else
		{
			V.TexCoord = glm::vec2{0.0f, 0.0f};
		}

		if (inAiMesh->HasTangentsAndBitangents())
		{
			V.Tangent.x = inAiMesh->mTangents[Index].x;
			V.Tangent.y = inAiMesh->mTangents[Index].y;
			V.Tangent.z = inAiMesh->mTangents[Index].z;

			V.Bitangent.x = inAiMesh->mBitangents[Index].x;
			V.Bitangent.y = inAiMesh->mBitangents[Index].y;
			V.Bitangent.z = inAiMesh->mBitangents[Index].z;
		}

		Vertices.push_back(V);
	}

	for (unsigned int FaceIndex = 0; FaceIndex < inAiMesh->mNumFaces; FaceIndex++)
	{
		aiFace& AiFace = inAiMesh->mFaces[FaceIndex];
		for (unsigned int Index = 0; Index < AiFace.mNumIndices; Index++)
		{
			Indices.push_back(AiFace.mIndices[Index]);
		}
	}

	std::shared_ptr<MeshData> OutMeshData = ObjectBase::NewObject<MeshData, std::string>(path + std::to_string(meshes.size()));
	OutMeshData->vertices = Vertices;
	OutMeshData->indices = Indices;

	return OutMeshData;
}
