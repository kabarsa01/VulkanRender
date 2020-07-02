#pragma once

#include <glm/glm.hpp>
#include "vulkan/vulkan.hpp"

// per frame update
struct alignas(16) ShaderGlobalData
{
	alignas(16) glm::mat4 worldToView;
	alignas(16) glm::mat4 viewToProj;
	alignas(16) glm::vec3 cameraPos;
	alignas(16) glm::vec3 viewVector;
	alignas(4) float time;
	alignas(4) float deltaTime;
	alignas(4) float cameraNear;
	alignas(4) float cameraFar;
	alignas(4) float cameraFov;
	alignas(4) float cameraAspect;
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
	alignas(16) LightInfo lights[1024];
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
	alignas(8) glm::u32vec2 clusters[32][32][64];
	alignas(4) glm::uint lightIndices[32][32][64][128];
};

