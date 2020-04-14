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
struct ObjectCommonData
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

