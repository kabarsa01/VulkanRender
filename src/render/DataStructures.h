#pragma once

#include <glm/glm.hpp>
#include "vulkan/vulkan.hpp"

// per frame update
struct ShaderGlobalData
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(4) float time;
	alignas(4) float deltaTime;
};

// per object update
struct ObjectMVPData
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

// per object update
struct TestData
{
	alignas(16) glm::vec4 color;
};

// cluster lights indices
struct ClusterLightsData
{
	alignas(16) glm::uint16 lightIndices[256];
};

