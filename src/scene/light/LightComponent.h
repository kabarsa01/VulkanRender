#pragma once

#include "scene/SceneObjectComponent.h"

#include <glm/ext/vector_float3.hpp>

class LightComponent : public SceneObjectComponent
{
public:
	glm::vec3 Color;

	LightComponent(std::shared_ptr<SceneObjectBase> InParent);
	virtual ~LightComponent();
};

typedef std::shared_ptr<LightComponent> LightComponentPtr;

