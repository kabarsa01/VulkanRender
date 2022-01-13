#pragma once

#include <glm/glm.hpp>
#include "vulkan/vulkan.hpp"

namespace CGE
{
	inline constexpr uint32_t g_GlobalTransformDataSize = 1024 * 1024;
	inline constexpr uint32_t g_LightsListSize = 1024;
	inline constexpr glm::u32vec3 g_ClusteringResolution = { 32,32,64 };
	inline constexpr uint32_t g_LightsPerCluster = 256;
	
	// per frame update
	struct alignas(16) GlobalShaderData
	{
		alignas(16) glm::mat4 worldToView;
		alignas(16) glm::mat4 viewToProj;
		alignas(16) glm::vec3 cameraPos;
		alignas(16) glm::vec3 viewVector;
		alignas(8) glm::uvec2 numClusters;
		alignas(8) glm::uvec2 clusterSize;
		alignas(8) glm::uvec2 halfScreenOffset;
		alignas(8) glm::vec2 clusterScreenOverflow;
		alignas(4) float time;
		alignas(4) float deltaTime;
		alignas(4) float cameraNear;
		alignas(4) float cameraFar;
		alignas(4) float cameraFov;
		alignas(4) float cameraAspect;
	};
	
	struct alignas(16) GlobalTransformData
	{
		alignas(16) glm::mat4 modelToWorld[g_GlobalTransformDataSize];
	};
	
	// per object update
	struct alignas(16) ObjectMVPData
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
	
	struct LightInfo
	{
		alignas(16) glm::vec4 position;
		alignas(16) glm::vec4 direction;
		alignas(16) glm::vec4 color;
		// radius, angle half, intensity
		alignas(16) glm::vec4 rai;
	};
	
	// cluster lights
	struct alignas(16) LightsList
	{
		alignas(16) LightInfo lights[g_LightsListSize];
	};
	
	struct alignas(16) LightsIndices
	{
		alignas(8) glm::u32vec2 directionalPosition;
		alignas(8) glm::u32vec2 spotPosition;
		alignas(8) glm::u32vec2 pointPosition;
	};
	
	// cluster lights indices
	struct alignas(16) ClusterLightsData
	{
		alignas(8) glm::u32vec2 clusters[g_ClusteringResolution.x][g_ClusteringResolution.y][g_ClusteringResolution.z];
		alignas(4) glm::uint lightIndices[g_ClusteringResolution.x][g_ClusteringResolution.y][g_ClusteringResolution.z][g_LightsPerCluster / 2];
	};

	// coordinate list
	struct alignas(16) CoordinateList
	{
		alignas(16) glm::vec4 coords[1000];
		alignas(4) glm::uint size;
	};

	// coordinate list
	struct alignas(16) GridHierarchy
	{
		alignas(16) glm::uvec4 gridSpecs;
		alignas(4) glm::uint lightsPerCell;
		alignas(8) vk::DeviceAddress grids;
	};
	
}
