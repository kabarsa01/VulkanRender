#pragma once

#include <glm/glm.hpp>
#include "vulkan/vulkan.hpp"

// per frame update
struct ShaderGlobalData
{
	alignas(16) glm::mat4 worldToView;
	alignas(16) glm::mat4 viewToProj;
	alignas(4) float time;
	alignas(4) float deltaTime;
	alignas(4) float cameraNear;
	alignas(4) float cameraFar;
};

// per object update
struct ObjectMVPData
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct LightInfo
{
	alignas(16) glm::vec3 position;
	alignas(16) glm::vec3 direction;
	alignas(16) glm::vec3 color;
	alignas(4) float intensity;
};

// cluster lights
struct LightsList
{
	alignas(8) glm::u32vec2 directionalPosition;
	alignas(8) glm::u32vec2 spotPosition;
	alignas(8) glm::u32vec2 pointPosition;
	alignas(16) LightInfo lights[4096];
};

// cluster lights indices
struct ClusterLightsData
{
	alignas(8) glm::u32vec2 clusters[32][32][32];
	alignas(4) glm::uint lightIndices[32][32][32][128];
};

