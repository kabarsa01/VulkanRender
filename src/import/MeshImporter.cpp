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
	const aiScene* scene = LocalImporter.ReadFile(
		inPath, 
		aiProcess_Triangulate
		| aiProcess_CalcTangentSpace
//		| aiProcess_GenSmoothNormals
//		| aiProcess_ValidateDataStructure
//		| aiProcess_FindDegenerates
		| aiProcess_FindInvalidData
		| aiProcess_MakeLeftHanded
		| aiProcess_FlipWindingOrder
//		| aiProcess_FlipUVs
	);

	if ( (scene == nullptr) || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || (scene->mRootNode == nullptr) )
	{
		std::cout << "ASSIMP::ERROR import " << LocalImporter.GetErrorString() << std::endl;

		if (scene != nullptr)
		{
//			delete Scene;
		}
		return;
	}

	ProcessNode(scene->mRootNode, scene);
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
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	for (unsigned int Index = 0; Index < inAiMesh->mNumVertices; Index++)
	{
		Vertex V;

		V.position.x = inAiMesh->mVertices[Index].x;
		V.position.y = inAiMesh->mVertices[Index].y;
		V.position.z = inAiMesh->mVertices[Index].z;

		V.normal.x = inAiMesh->mNormals[Index].x;
		V.normal.y = inAiMesh->mNormals[Index].y;
		V.normal.z = inAiMesh->mNormals[Index].z;

		// we use only one set of texture coordinates for one
		if (inAiMesh->HasTextureCoords(0))
		{
			V.texCoord.x = inAiMesh->mTextureCoords[0][Index].x;
			V.texCoord.y = inAiMesh->mTextureCoords[0][Index].y;
		}
		else
		{
			V.texCoord = glm::vec2{0.0f, 0.0f};
		}

		if (inAiMesh->HasTangentsAndBitangents())
		{
			V.tangent.x = inAiMesh->mTangents[Index].x;
			V.tangent.y = inAiMesh->mTangents[Index].y;
			V.tangent.z = inAiMesh->mTangents[Index].z;

			V.bitangent.x = inAiMesh->mBitangents[Index].x;
			V.bitangent.y = inAiMesh->mBitangents[Index].y;
			V.bitangent.z = inAiMesh->mBitangents[Index].z;
		}

		vertices.push_back(V);
	}

	for (unsigned int FaceIndex = 0; FaceIndex < inAiMesh->mNumFaces; FaceIndex++)
	{
		aiFace& AiFace = inAiMesh->mFaces[FaceIndex];

		//Vertex v0 = vertices[AiFace.mIndices[0]];
		//Vertex v1 = vertices[AiFace.mIndices[1]];
		//Vertex v2 = vertices[AiFace.mIndices[2]];

		//CalculateTangents(v0, v1, v2);
		//CalculateTangents(v1, v2, v0);
		//CalculateTangents(v2, v0, v1);

		//vertices[AiFace.mIndices[0]] = v0;
		//vertices[AiFace.mIndices[1]] = v1;
		//vertices[AiFace.mIndices[2]] = v2;

		for (unsigned int Index = 0; Index < AiFace.mNumIndices; Index++)
		{
			indices.push_back(AiFace.mIndices[Index]);
		}
	}

	std::shared_ptr<MeshData> OutMeshData = ObjectBase::NewObject<MeshData, std::string>(path + std::to_string(meshes.size()));
	OutMeshData->vertices = vertices;
	OutMeshData->indices = indices;

	return OutMeshData;
}

void MeshImporter::CalculateTangents(Vertex& v0, const Vertex& v1, const Vertex& v2)
{
	glm::vec3 edge1 = (v1.position - v0.position) * 100.0f;
	glm::vec3 edge2 = (v1.position - v2.position) * 100.0f;

	glm::vec2 uv0 = { v0.texCoord.x, v0.texCoord.y };
	glm::vec2 uv1 = { v1.texCoord.x, v1.texCoord.y };
	glm::vec2 uv2 = { v2.texCoord.x, v2.texCoord.y };

	glm::vec2 deltaUV1 = (uv1 - uv0) * 100.0f;
	glm::vec2 deltaUV2 = (uv1 - uv2) * 100.0f;

	//deltaUV1 *= glm::vec2(-1.0, -1.0);
	//deltaUV2 *= glm::vec2(-1.0, -1.0);

	float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

	//glm::vec3 t = glm::normalize(edge1 / deltaUV1.x);
	//glm::vec3 b = glm::cross(v0.normal, t);

	//t = glm::cross(b, v0.normal);

	v0.tangent = glm::normalize<3>((edge1 * deltaUV2.y - edge2 * deltaUV1.y) * r);
	v0.bitangent = glm::normalize<3>((edge2 * deltaUV1.x - edge1 * deltaUV2.x) * r);
}
