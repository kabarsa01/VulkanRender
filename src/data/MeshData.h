#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "data/Resource.h"
#include "vulkan/vulkan.hpp"

using namespace std;
using namespace glm;

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

	static std::array<VULKAN_HPP_NAMESPACE::VertexInputAttributeDescription, 5> GetAttributeDescriptions();
};


class MeshData : public Resource
{
public:
	vector<Vertex> vertices;
	vector<unsigned int> indices;

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

	VULKAN_HPP_NAMESPACE::VertexInputBindingDescription GetBindingDescription();
	// fullscreen quad instance to be used for screen space stuff
	static std::shared_ptr<MeshData> FullscreenQuad();
private:
	VULKAN_HPP_NAMESPACE::Buffer vertexBuffer;
	VULKAN_HPP_NAMESPACE::DeviceMemory vertexBufferMemory;

	MeshData() : Resource(HashString::NONE()) {}
	uint32_t FindMemoryType(uint32_t inTypeFilter, VULKAN_HPP_NAMESPACE::MemoryPropertyFlags inPropFlags);
};

typedef std::shared_ptr<MeshData> MeshDataPtr;

