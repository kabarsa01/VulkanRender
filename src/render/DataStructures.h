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
	alignas(4) float cameraFov;
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
	alignas(16) glm::vec4 position;
	alignas(16) glm::vec4 direction;
	alignas(16) glm::vec4 color;
	// radius, angle half, intensity
	alignas(16) glm::vec4 rai;
};

// cluster lights
struct LightsList
{
	alignas(16) LightInfo lights[1024];
};

struct LightsIndices
{
	alignas(8) glm::u32vec2 directionalPosition;
	alignas(8) glm::u32vec2 spotPosition;
	alignas(8) glm::u32vec2 pointPosition;
};

// cluster lights indices
struct ClusterLightsData
{
	alignas(8) glm::u32vec2 clusters[32][32][64];
	alignas(4) glm::uint lightIndices[32][32][64][128];
};

