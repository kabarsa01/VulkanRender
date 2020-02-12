#include "import/MeshImporter.h"
#include "core/ObjectBase.h"

#include <iostream>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>


void MeshImporter::Import(std::string InPath)
{
	Path = InPath;
	Assimp::Importer LocalImporter;
	const aiScene* Scene = LocalImporter.ReadFile(InPath, aiProcess_Triangulate | aiProcess_CalcTangentSpace);// | aiProcess_FlipUVs);

	if ( (Scene == nullptr) || (Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || (Scene->mRootNode == nullptr) )
	{
		cout << "ASSIMP::ERROR import " << LocalImporter.GetErrorString() << endl;

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
	return Meshes;
}

void MeshImporter::ProcessNode(aiNode * Node, const aiScene * Scene)
{
	for (unsigned int MeshIndex = 0; MeshIndex < Node->mNumMeshes; MeshIndex++)
	{
		aiMesh* Mesh = Scene->mMeshes[Node->mMeshes[MeshIndex]];
		Meshes.push_back(ProcessMesh(Mesh));
	}

	// recursive children processing
	for (unsigned int ChildIndex = 0; ChildIndex < Node->mNumChildren; ChildIndex++)
	{
		ProcessNode(Node->mChildren[ChildIndex], Scene);
	}
}

std::shared_ptr<MeshData> MeshImporter::ProcessMesh(aiMesh* AiMesh)
{
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;

	for (unsigned int Index = 0; Index < AiMesh->mNumVertices; Index++)
	{
		Vertex V;

		V.Position.x = AiMesh->mVertices[Index].x;
		V.Position.y = AiMesh->mVertices[Index].y;
		V.Position.z = AiMesh->mVertices[Index].z;

		V.Normal.x = AiMesh->mNormals[Index].x;
		V.Normal.y = AiMesh->mNormals[Index].y;
		V.Normal.z = AiMesh->mNormals[Index].z;

		// we use only one set of texture coordinates for one
		if (AiMesh->HasTextureCoords(0))
		{
			V.TexCoord.x = AiMesh->mTextureCoords[0][Index].x;
			V.TexCoord.y = AiMesh->mTextureCoords[0][Index].y;
		}
		else
		{
			V.TexCoord = glm::vec2{0.0f, 0.0f};
		}

		if (AiMesh->HasTangentsAndBitangents())
		{
			V.Tangent.x = AiMesh->mTangents[Index].x;
			V.Tangent.y = AiMesh->mTangents[Index].y;
			V.Tangent.z = AiMesh->mTangents[Index].z;

			V.Bitangent.x = AiMesh->mBitangents[Index].x;
			V.Bitangent.y = AiMesh->mBitangents[Index].y;
			V.Bitangent.z = AiMesh->mBitangents[Index].z;
		}

		Vertices.push_back(V);
	}

	for (unsigned int FaceIndex = 0; FaceIndex < AiMesh->mNumFaces; FaceIndex++)
	{
		aiFace& AiFace = AiMesh->mFaces[FaceIndex];
		for (unsigned int Index = 0; Index < AiFace.mNumIndices; Index++)
		{
			Indices.push_back(AiFace.mIndices[Index]);
		}
	}

	std::shared_ptr<MeshData> OutMeshData = ObjectBase::NewObject<MeshData, std::string>(Path + std::to_string(Meshes.size()));
	OutMeshData->Vertices = Vertices;
	OutMeshData->Indices = Indices;

	return OutMeshData;
}
